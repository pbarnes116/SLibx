#include "../../../inc/slibx/sdev/sapp.h"

#include "../../../inc/slibx/sdev/util.h"

#include <slib/core/file.h>
#include <slib/core/log.h>

#define TAG "SApp"

SLIB_SDEV_NAMESPACE_BEGIN

SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_begin, "Opening app configuration file : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_end, "Finished app configuration")
SLIB_STATIC_STRING(_g_sdev_sapp_log_resource_begin, "Opening resource file : %s")

SLIB_STATIC_STRING(_g_sdev_sapp_error_out_of_memory, "Out of memory")
SLIB_STATIC_STRING(_g_sdev_sapp_error_file_not_found, "File not found: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_file_write_failed, "Failed to write to file : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_invalid_root_tag, "<sapp> tag is not declared for root element")
SLIB_STATIC_STRING(_g_sdev_sapp_error_invalid_tag, "Invalid tag : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_string_name_invalid, "The name of string resource is invalid : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_string_not_found, "String resource is not defined : %s")

SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_locale_invalid, "String Resource: Invalid locale attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_name_is_empty, "String Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_name_invalid, "String Resource: Invalid name attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_redefine_default, "String Resource: default value is redefined")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_redefine_locale, "String Resource: value is redefined for %s locale")

SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_name_is_empty, "Menu Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_name_invalid, "Menu Resource: Invalid name attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_name_redefined, "Menu Resource: menu name is redefined : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_platform_invalid, "Menu Resource: Invalid platform attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_shortcutKey_invalid, "Menu Resource: Invalid shortcutKey attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_macShortcutKey_invalid, "Menu Resource: Invalid macShortcutKey attribute value : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_children_tag_invalid, "Menu Resource: Invalid tag : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_item_name_redefined, "Menu Resource: item name is redefined : %s")

SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_begin, "Generating C++ files : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_target_path_is_empty, "App configuration: <generate-cpp>/<target-path> is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_target_path_invalid, "App configuration: Failed to create target path for generating cpp : %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_namespace_is_empty, "App configuration: <generate-cpp>/<namespace> is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_namespace_invalid, "App configuration: Invalid <generate-cpp>/<namespace>: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_strings_begin, "Generating: strings.h, strings.cpp")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_menus_begin, "Generating: menus.h, menus.cpp")

SAppDocument::SAppDocument()
{
	m_flagOpened = sl_false;
}

SAppDocument::~SAppDocument()
{
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
		_logError(_g_sdev_sapp_error_generate_cpp_target_path_is_empty);
		return sl_false;
	}
	if (!(File::isDirectory(path))) {
		_logError(_g_sdev_sapp_error_generate_cpp_target_path_invalid.arg(path));
		return sl_false;
	}
	
	if (m_conf.generate_cpp_namespace.isEmpty()) {
		_logError(_g_sdev_sapp_error_generate_cpp_namespace_is_empty);
		return sl_false;
	}
	if (!(SDevUtil::checkName(m_conf.generate_cpp_namespace.getData(), m_conf.generate_cpp_namespace.getLength()))) {
		_logError(_g_sdev_sapp_error_generate_cpp_namespace_invalid.arg(m_conf.generate_cpp_namespace));
		return sl_false;
	}
	
	_log(_g_sdev_sapp_log_generate_cpp_begin.arg(path));
	if (!_generateStringsCpp(path)) {
		return sl_false;
	}
	if (!_generateMenusCpp(path)) {
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
			} else if (child->getName() == "menu") {
			} else {
				_logError(child, _g_sdev_sapp_error_invalid_tag.arg(child->getName()));
				return sl_false;
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
			} else {
				_logError(child, _g_sdev_sapp_error_invalid_tag.arg(child->getName()));
				return sl_false;
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
	
	Ref<SAppMenuResource> menu = new SAppMenuResource;
	if (menu.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	String name = element->getAttribute("name");
	if (name.isEmpty()) {
		_logError(element, _g_sdev_sapp_error_resource_menu_name_is_empty);
		return sl_false;
	}
	if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
		_logError(element, _g_sdev_sapp_error_resource_menu_name_invalid.arg(name));
		return sl_false;
	}
	menu->name = name;
	
	ListLocker< Ref<XmlElement> > children(element->getChildElements());
	for (sl_size i = 0; i < children.count; i++) {
		Ref<XmlElement>& child = children[i];
		if (child.isNotNull()) {
			Ref<SAppMenuResourceItem> menuItem = _parseMenuResourceItem(child, menu.ptr, SAppMenuResourceItem::all_platforms);
			if (menuItem.isNull()) {
				return sl_false;
			}
			if (!(menu->children.add(menuItem))) {
				_logError(element, _g_sdev_sapp_error_out_of_memory);
				return sl_false;
			}
		}
	}
	
	if (m_menus.contains(name)) {
		_logError(element, _g_sdev_sapp_error_resource_menu_name_redefined.arg(name));
		return sl_false;
	}
	
	if (!(m_menus.put(name, menu))) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	return sl_true;
}

Ref<SAppMenuResourceItem> SAppDocument::_parseMenuResourceItem(const Ref<XmlElement>& element, SAppMenuResource* menu, int parentPlatforms)
{
	if (element.isNull()) {
		return Ref<SAppMenuResourceItem>::null();
	}
	
	Ref<SAppMenuResourceItem> item = new SAppMenuResourceItem;
	if (item.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return Ref<SAppMenuResourceItem>::null();
	}
	
	if (element->getName() == "submenu") {
		item->type = SAppMenuResourceItem::typeSubmenu;
	} else if (element->getName() == "item") {
		item->type = SAppMenuResourceItem::typeItem;
	} else if (element->getName() == "separator") {
		item->type = SAppMenuResourceItem::typeSeparator;
	} else {
		_logError(element, _g_sdev_sapp_error_resource_menu_children_tag_invalid.arg(element->getName()));
		return Ref<SAppMenuResourceItem>::null();
	}
	
	String name = element->getAttribute("name");
	if (name.isEmpty()) {
		if (item->type != SAppMenuResourceItem::typeSeparator) {
			_logError(element, _g_sdev_sapp_error_resource_menu_name_is_empty);
			return Ref<SAppMenuResourceItem>::null();
		}
	} else {
		if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
			_logError(element, _g_sdev_sapp_error_resource_menu_name_invalid.arg(name));
			return Ref<SAppMenuResourceItem>::null();
		}
	}
	item->name = name;
	
	String strPlatform = element->getAttribute("platform");
	if (strPlatform.isEmpty()) {
		item->platformFlags = parentPlatforms;
	} else {
		if (strPlatform == "no-mac") {
			item->platformFlags = SAppMenuResourceItem::no_mac;
		} else if (strPlatform == "no-windows") {
			item->platformFlags = SAppMenuResourceItem::no_windows;
		} else if (strPlatform == "no-linux") {
			item->platformFlags = SAppMenuResourceItem::no_linux;
		} else {
			item->platformFlags = 0;
			if (strPlatform.contains("mac")) {
				item->platformFlags |= SAppMenuResourceItem::mac;
			}
			if (strPlatform.contains("windows")) {
				item->platformFlags |= SAppMenuResourceItem::windows;
			}
			if (strPlatform.contains("linux")) {
				item->platformFlags |= SAppMenuResourceItem::linux;
			}
		}
		item->platformFlags &= parentPlatforms;
	}
	if (item->platformFlags == 0) {
		_logError(element, _g_sdev_sapp_error_resource_menu_platform_invalid.arg(strPlatform));
		return Ref<SAppMenuResourceItem>::null();
	}
	
	if (name.isNotEmpty()) {
		if (item->platformFlags & SAppMenuResourceItem::mac) {
			if (menu->itemsMac.contains(name)) {
				_logError(element, _g_sdev_sapp_error_resource_menu_item_name_redefined.arg(name));
				return Ref<SAppMenuResourceItem>::null();
			}
			if (!(menu->itemsMac.put(name, item))) {
				_logError(element, _g_sdev_sapp_error_out_of_memory);
				return Ref<SAppMenuResourceItem>::null();
			}
		}
		if (item->platformFlags & SAppMenuResourceItem::windows) {
			if (menu->itemsWindows.contains(name)) {
				_logError(element, _g_sdev_sapp_error_resource_menu_item_name_redefined.arg(name));
				return Ref<SAppMenuResourceItem>::null();
			}
			if (!(menu->itemsWindows.put(name, item))) {
				_logError(element, _g_sdev_sapp_error_out_of_memory);
				return Ref<SAppMenuResourceItem>::null();
			}
		}
		if (item->platformFlags & SAppMenuResourceItem::linux) {
			if (menu->itemsLinux.contains(name)) {
				_logError(element, _g_sdev_sapp_error_resource_menu_item_name_redefined.arg(name));
				return Ref<SAppMenuResourceItem>::null();
			}
			if (!(menu->itemsLinux.put(name, item))) {
				_logError(element, _g_sdev_sapp_error_out_of_memory);
				return Ref<SAppMenuResourceItem>::null();
			}
		}
	}

	if (item->type != SAppMenuResourceItem::typeSeparator) {
		String title = element->getAttribute("title");
		item->title.flagReferResource = sl_false;
		if (title == "@") {
			item->title.flagReferResource = sl_true;
			title = menu->name + "_" + name;
		} else if (title.startsWith("@")) {
			title = title.substring(1);
			if (!(title.startsWith("@"))) {
				if (!(SDevUtil::checkName(title.getData(), title.getLength()))) {
					_logError(element, _g_sdev_sapp_error_string_name_invalid.arg(title));
					return Ref<SAppMenuResourceItem>::null();
				}
				item->title.flagReferResource = sl_true;
			}
		}
		item->title.value = title;
		if (item->title.flagReferResource) {
			if (!(m_strings.contains(item->title.value))) {
				_logError(element, _g_sdev_sapp_error_string_not_found.arg(item->title.value));
				return Ref<SAppMenuResourceItem>::null();
			}
		}
	}
	
	if (item->type == SAppMenuResourceItem::typeItem) {
		String strShortcutKey = element->getAttribute("shortcutKey");
		if (!(item->shortcutKey.parse(strShortcutKey))) {
			_logError(element, _g_sdev_sapp_error_resource_menu_shortcutKey_invalid.arg(strShortcutKey));
			return Ref<SAppMenuResourceItem>::null();
		}
		strShortcutKey = element->getAttribute("macShortcutKey");
		if (!(item->macShortcutKey.parse(strShortcutKey))) {
			_logError(element, _g_sdev_sapp_error_resource_menu_macShortcutKey_invalid.arg(strShortcutKey));
			return Ref<SAppMenuResourceItem>::null();
		}
	}
	
	if (item->type == SAppMenuResourceItem::typeSubmenu) {
		ListLocker< Ref<XmlElement> > children(element->getChildElements());
		for (sl_size i = 0; i < children.count; i++) {
			Ref<XmlElement>& child = children[i];
			if (child.isNotNull()) {
				Ref<SAppMenuResourceItem> menuItem = _parseMenuResourceItem(child, menu, item->platformFlags);
				if (menuItem.isNull()) {
					return Ref<SAppMenuResourceItem>::null();
				}
				if (!(item->children.add(menuItem))) {
					_logError(element, _g_sdev_sapp_error_out_of_memory);
					return Ref<SAppMenuResourceItem>::null();
				}
			}
		}
	}
	
	return item;
}

sl_bool SAppDocument::_generateStringsCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_strings_begin);
	
	StringBuffer sbHeader, sbCpp;
	
	sbHeader.add(String::format(
								"#ifndef CHECKHEADER_RESOURCE_STRINGS_%s%n"
								"#define CHECKHEADER_RESOURCE_STRINGS_%s%n%n"
								"#include <slib/core/resource.h>%n%n"
								"namespace %s%n"
								"{%n\tnamespace strings%n\t{%n"
								, m_conf.generate_cpp_namespace));
	sbCpp.add(String::format(
							 "#include \"strings.h\"%n%n"
							 "namespace %s%n"
							 "{%n\tnamespace strings%n\t{%n"
							 , m_conf.generate_cpp_namespace));
	
	String strTab('\t', 2);
	
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
	
	sbHeader.add("\t}\r\n}\r\n\r\n#endif");
	sbCpp.add("\t}\r\n}\r\n");

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

sl_bool SAppDocument::_generateMenusCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_menus_begin);
	
	StringBuffer sbHeader, sbCpp;
	sbHeader.add(String::format(
								"#ifndef CHECKHEADER_RESOURCE_MENUS_%s%n"
								"#define CHECKHEADER_RESOURCE_MENUS_%s%n%n"
								"#include <slib/ui/menu.h>%n%n"
								"namespace %s%n"
								"{%n\tnamespace menus%n\t{%n"
								, m_conf.generate_cpp_namespace));
	
	sbCpp.add(String::format(
							 "#include \"menus.h\"%n"
							 "#include \"strings.h\"%n%n"
							 "namespace %s%n"
							 "{%n\tnamespace menus%n\t{%n"
							 , m_conf.generate_cpp_namespace));
	
	
	
	
	sbHeader.add("\t}\r\n}\r\n\r\n#endif");
	sbCpp.add("\t}\r\n}\r\n");
	
	String pathHeader = targetPath + "/menus.h";
	String contentHeader = sbHeader.merge();
	if (!(File::writeAllTextUTF8(pathHeader, contentHeader))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathHeader));
		return sl_false;
	}
	
	String pathCpp = targetPath + "/menus.cpp";
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

void SAppDocument::_logError(sl_size line, sl_size col, const String& text)
{
	SLIB_LOG_ERROR(TAG, String::format("%s(%d:%d)%n%s", m_lastFilePath, line, col, text));
}

void SAppDocument::_logError(const Ref<XmlElement>& element, const String& text)
{
	SLIB_LOG_ERROR(TAG, String::format("%s(%d:%d)%n%s", m_lastFilePath, element->getLineNumberInSource(), element->getColumnNumberInSource(), text));
}

SLIB_SDEV_NAMESPACE_END
