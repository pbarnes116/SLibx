#include "../../../inc/slibx/sdev/sapp.h"

#include "../../../inc/slibx/sdev/util.h"

#include <slib/core/file.h>
#include <slib/core/log.h>

#define TAG "SApp"

SLIB_SDEV_NAMESPACE_BEGIN

SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_begin, "Opening app configuration file : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_end, "Finished app configuration")
SLIB_STATIC_STRING(_g_sdev_sapp_log_resource_begin, "Opening resource file : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_begin, "Generating C++ files : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_strings_begin, "Generating: strings.h, strings.cpp")

SLIB_STATIC_STRING(_g_sdev_sapp_error_out_of_memory, "Out of memory")
SLIB_STATIC_STRING(_g_sdev_sapp_error_file_not_found, "File not found: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_file_open_for_write_failed, "Failed to open file for write: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_file_write_failed, "Failed to write to file : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_invalid_root_tag, "<sapp> tag is not declared for root element")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_locale_invalid, "String Resource: Invalid locale attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_name_is_empty, "String Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_name_invalid, "String Resource: Invalid name attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_redefine_default, "String Resource: default value is redefined")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_redefine_locale, "String Resource: value is redefined for %s locale")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_target_path_not_defined, "<generate-cpp>/<target-path> is not defined in app configuration")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_target_path_is_empty, "<generate-cpp>/<target-path> is empty value")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_target_path_invalid, "Failed to create target path for generating cpp : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_namespace_invalid, "Invalid namespace: %s")

SAppDocument::SAppDocument()
{
	m_flagOpened = sl_false;
}

sl_bool SAppDocument::open(const String& filePath)
{
	ObjectLocker lock(this);
	
	SAppConfiguration conf;
	if (!_parseConfiguration(filePath, conf)) {
		return sl_false;
	}
	
	close();
	
	m_pathConf = filePath;
	m_pathApp = File::getParentDirectoryPath(filePath);
	
	m_conf = conf;
	
	m_flagOpened = sl_true;
	
	return sl_true;
}

void SAppDocument::close()
{
	ObjectLocker lock(this);
	if (m_flagOpened) {
		_freeResources();
		m_flagOpened = sl_false;
	}
}

sl_bool SAppDocument::isOpened()
{
	return m_flagOpened;
}

sl_bool SAppDocument::openResources()
{
	ObjectLocker lock(this);
	
	if (!m_flagOpened) {
		return sl_false;
	}
	
	_freeResources();
	
	ListLocker<String> list(File::getFiles(m_pathApp + "/values"));
	for (sl_size i = 0; i < list.count; i++) {
		const String& file = list[i];
		if (file.isNotNull()) {
			String path = m_pathApp + "/values/" + file;
			if (File::exists(path)) {
				if (!(File::isDirectory(path))) {
					if (File::getFileExtension(file) == "xml") {
						if (!(_parseResources(path))) {
							return sl_false;
						}
					}
				}
			}
		}
	}
	return sl_true;
}

sl_bool SAppDocument::generateCpp()
{
	ObjectLocker lock(this);

	if (!m_flagOpened) {
		return sl_false;
	}
	
	String path = m_conf.generate_cpp_target_path;
	if (path.isEmpty()) {
		_logError(_g_sdev_sapp_error_generate_cpp_target_path_not_defined);
		return sl_false;
	}
	if (!(File::isDirectory(path))) {
		_logError(_g_sdev_sapp_error_generate_cpp_target_path_invalid.arg(path));
		return sl_false;
	}
	
	_log(_g_sdev_sapp_log_generate_cpp_begin.arg(path));
	if (!_generateStringsCpp(path)) {
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseConfiguration(const String& filePath, SAppConfiguration& conf)
{
	_log(_g_sdev_sapp_log_appconf_begin.arg(filePath));
	
	if (!(File::exists(filePath))) {
		_logError(_g_sdev_sapp_error_file_not_found.arg(filePath));
		return sl_false;
	}
	
	m_lastFilePath = filePath;
	XmlParseParam param;
	param.setCreatingOnlyElementsAndTexts();
	Ref<XmlDocument> xml = Xml::parseXmlFromTextFile(filePath, param);
	if (param.flagError) {
		_logError(param.errorLine, param.errorColumn, param.errorMessage);
		return sl_false;
	}
	
	if (xml.isNull()) {
		return sl_false;
	}
	Ref<XmlElement> root = xml->getRoot();
	if (root.isNull()) {
		return sl_false;
	}
	if (root->getName() != "sapp") {
		_logError(root, _g_sdev_sapp_error_invalid_root_tag);
		return sl_false;
	}
	
	// generate-cpp
	Ref<XmlElement> el_generate_cpp = root->getFirstChildElement("generate-cpp");
	if (el_generate_cpp.isNotNull()) {
		Ref<XmlElement> el_target_path = el_generate_cpp->getFirstChildElement("target-path");
		if (el_target_path.isNotNull()) {
			String strPath = el_target_path->getText();
			if (strPath.isEmpty()) {
				_logError(el_target_path, _g_sdev_sapp_error_generate_cpp_target_path_is_empty);
				return sl_false;
			}
			if (strPath.startsWith('.')) {
				strPath = File::getParentDirectoryPath(filePath) + "/" + strPath;
			}
			conf.generate_cpp_target_path = strPath;
		}
		Ref<XmlElement> el_namespace = el_generate_cpp->getFirstChildElement("namespace");
		if (el_namespace.isNotNull()) {
			conf.generate_cpp_namespace = el_namespace->getText();
			if (!(SDevUtil::checkName(conf.generate_cpp_namespace.getData(), conf.generate_cpp_namespace.getLength()))) {
				_logError(el_namespace, _g_sdev_sapp_error_generate_cpp_namespace_invalid.arg(conf.generate_cpp_namespace));
				return sl_false;
			}
		}
	}
	
	_log(_g_sdev_sapp_log_appconf_end);
	
	return sl_true;
}

void SAppDocument::_freeResources()
{
	m_strings.removeAll();
}

sl_bool SAppDocument::_parseResources(const String& filePath)
{
	_log(_g_sdev_sapp_log_resource_begin.arg(filePath));
	
	m_lastFilePath = filePath;
	
	XmlParseParam param;
	param.setCreatingOnlyElementsAndTexts();
	Ref<XmlDocument> xml = Xml::parseXmlFromTextFile(filePath, param);
	if (param.flagError) {
		_logError(param.errorLine, param.errorColumn, param.errorMessage);
		return sl_false;
	}
	if (xml.isNull()) {
		return sl_false;
	}
	Ref<XmlElement> root = xml->getRoot();
	if (root.isNull()) {
		return sl_false;
	}
	if (root->getName() != "sapp") {
		_logError(root, _g_sdev_sapp_error_invalid_root_tag);
		return sl_false;
	}
	ListLocker< Ref<XmlElement> > children(root->getChildElements());
	sl_size i;
	for (i = 0; i < children.count; i++) {
		Ref<XmlElement>& child = children[i];
		if (child.isNotNull()) {
			if (child->getName() == "strings") {
				if (!_parseStringResources(child)) {
					return sl_false;
				}
			} else if (child->getName() == "string") {
				if (!_parseStringResource(child, Locale::Unknown)) {
					return sl_false;
				}
			}
		}
	}
	for (i = 0; i < children.count; i++) {
		Ref<XmlElement>& child = children[i];
		if (child.isNotNull()) {
			if (child->getName() == "menu") {
				if (!_parseMenuResource(child)) {
					return sl_false;
				}
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseStringResources(const Ref<XmlElement>& element)
{
	if (element.isNull()) {
		return sl_false;
	}
	String strLocale = element->getAttribute("locale");
	Locale locale = Locale::Unknown;
	if (strLocale.isNotEmpty()) {
		locale = Locale(strLocale);
		if (locale.isInvalid()) {
			_logError(element, _g_sdev_sapp_error_resource_string_locale_invalid.arg(strLocale));
			return sl_false;
		}
	}
	ListLocker< Ref<XmlElement> > children(element->getChildElements());
	for (sl_size i = 0; i < children.count; i++) {
		Ref<XmlElement>& child = children[i];
		if (child.isNotNull()) {
			if (child->getName() == "string") {
				if (!_parseStringResource(child, locale)) {
					return sl_false;
				}
			}
		}
	}
	return sl_true;
}

sl_bool SAppDocument::_parseStringResource(const Ref<XmlElement>& element, Locale localeDefault)
{
	if (element.isNull()) {
		return sl_false;
	}
	
	String strLocale = element->getAttribute("locale");
	Locale locale = Locale::Unknown;
	if (strLocale.isNotEmpty()) {
		locale = Locale(strLocale);
		if (locale.isInvalid()) {
			_logError(element, _g_sdev_sapp_error_resource_string_locale_invalid.arg(strLocale));
			return sl_false;
		}
	}
	if (locale == Locale::Unknown) {
		locale = localeDefault;
	}
	
	String name = element->getAttribute("name");
	if (name.isEmpty()) {
		_logError(element, _g_sdev_sapp_error_resource_string_name_is_empty);
		return sl_false;
	}
	if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
		_logError(element, _g_sdev_sapp_error_resource_string_name_invalid.arg(name));
		return sl_false;
	}
	
	String value = element->getText().getNotNull();
	
	Ref<SAppStringResource> res = m_strings.getValue(name, Ref<SAppStringResource>::null());
	if (res.isNull()) {
		res = new SAppStringResource;
		if (res.isNull()) {
			_logError(element, _g_sdev_sapp_error_out_of_memory);
			return sl_false;
		}
		res->name = name;
		if (!(m_strings.put(name, res))) {
			_logError(element, _g_sdev_sapp_error_out_of_memory);
			return sl_false;
		}
	}
	
	if (locale == Locale::Unknown) {
		if (res->defaultValue.isNotNull()) {
			_logError(element, _g_sdev_sapp_error_resource_string_redefine_default);
			return sl_false;
		}
		res->defaultValue = value;
	} else {
		if (res->values.contains(locale)) {
			_logError(element, _g_sdev_sapp_error_resource_string_redefine_locale.arg(locale.toString()));
			return sl_false;
		}
		if (!(res->values.put(locale, value))) {
			_logError(element, _g_sdev_sapp_error_out_of_memory);
			return sl_false;
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseMenuResource(const Ref<XmlElement>& element)
{
	if (element.isNull()) {
		return sl_false;
	}
	
	
	
	return sl_true;
}

sl_bool SAppDocument::_generateStringsCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_strings_begin);
	
	String beginHeader, endHeader, beginCpp, endCpp;
	sl_uint32 tabLevel;
	if (m_conf.generate_cpp_namespace.isNotEmpty()) {
		beginHeader = String::format(
							   "#ifndef CHECKHEADER_RESOURCE_STRINGS_%s%n"
							   "#define CHECKHEADER_RESOURCE_STRINGS_%s%n%n"
							   "#include <slib/core/resource.h>%n%n"
							   "namespace %s%n"
							   "{%n\tnamespace strings%n\t{%n"
							   , m_conf.generate_cpp_namespace);
		endHeader = "\t}\r\n}\r\n\r\n#endif";
		beginCpp = String::format(
							   "#include \"strings.h\"%n%n"
							   "namespace %s%n"
							   "{%n\tnamespace strings%n\t{%n"
							   , m_conf.generate_cpp_namespace);
		endCpp = "\t}\r\n}\r\n";
		tabLevel = 2;
	} else {
		beginHeader = String::format(
							   "#ifndef CHECKHEADER_RESOURCE_STRINGS_%s%n"
							   "#define CHECKHEADER_RESOURCE_STRINGS_%s%n%n"
							   "#include <slib/core/resource.h>%n%n"
							   "namespace strings%n{%n"
							   , m_conf.generate_cpp_namespace);
		endHeader = "}\r\n\r\n#endif";
		beginCpp = String::format(
							   "#include \"strings.h\"%n%n"
							   "namespace strings%n{%n"
							   , m_conf.generate_cpp_namespace);
		endCpp = "}\r\n";
		tabLevel = 1;
	}
	
	StringBuffer sbHeader, sbCpp;
	sbHeader.add(beginHeader);
	sbCpp.add(beginCpp);
	
	String strTab('\t', tabLevel);
	
	Iterator< Pair<String, Ref<SAppStringResource> > > iterator = m_strings.iterator();
	Pair< String, Ref<SAppStringResource> > pair;
	
	while (iterator.next(&pair)) {
		if (pair.value.isNotNull()) {
			
			sbHeader.add(strTab);
			sbHeader.add(String::format("SLIB_DECLARE_STRING_RESOURCE(%s)%n", pair.key));
			
			String defValue = pair.value->defaultValue;
			sbCpp.add(strTab);
			sbCpp.add(String::format("SLIB_BEGIN_STRING_RESOURCE(%s, \"%s\")%n", pair.key, defValue.applyBackslashEscapes(sl_true, sl_false, sl_true)));
			
			Iterator< Pair<Locale, String> > iteratorValues = pair.value->values.iterator();
			Pair< Locale, String > pairValues;
			while (iteratorValues.next(&pairValues)) {
				sbCpp.add(strTab);
				sbCpp.addStatic(strTab.getData(), 1);
				sbCpp.add(String::format("SLIB_DEFINE_STRING_RESOURCE_VALUE(%s, \"%s\")%n", pairValues.key.toString(), pairValues.value.applyBackslashEscapes(sl_true, sl_false, sl_true)));
			}
			
			static sl_char8 strEnd[] = "SLIB_END_STRING_RESOURCE\r\n";
			sbCpp.add(strTab);
			sbCpp.addStatic(strEnd, sizeof(strEnd)-1);
		}
	}
	
	sbHeader.add(endHeader);
	sbCpp.add(endCpp);

	String pathHeader = targetPath + "/strings.h";
	String contentHeader = sbHeader.merge();
	if (!(File::writeAllTextUTF8(pathHeader, contentHeader))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathHeader));
		return sl_false;
	}
	
	String pathCpp = targetPath + "/strings.cpp";
	String contentCpp = sbCpp.merge();
	if (!(File::writeAllTextUTF8(pathCpp, contentCpp))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathCpp));
		return sl_false;
	}
	
	return sl_true;
}

void SAppDocument::_log(const String& text)
{
	SLIB_LOG(TAG, text);
}

void SAppDocument::_logError(const String& text)
{
	SLIB_LOG_ERROR(TAG, text);
}

void SAppDocument::_logError(sl_uint32 line, sl_uint32 col, const String& text)
{
	SLIB_LOG_ERROR(TAG, String::format("%s(%d:%d)%n%s", m_lastFilePath, line, col, text));
}

void SAppDocument::_logError(const Ref<XmlElement>& element, const String& text)
{
	SLIB_LOG_ERROR(TAG, String::format("%s(%d:%d)%n%s", m_lastFilePath, element->getLineNumberInSource(), element->getColumnNumberInSource(), text));
}

SLIB_SDEV_NAMESPACE_END
