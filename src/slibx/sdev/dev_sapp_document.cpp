#include "../../../inc/slibx/sdev/sapp.h"

#include "../../../inc/slibx/sdev/util.h"

#include <slib/core/file.h>
#include <slib/core/log.h>
#include <slib/core/resource.h>
#include <slib/core/scoped_pointer.h>
#include <slib/ui/core.h>

#define TAG "SApp"

SLIB_SDEV_NAMESPACE_BEGIN

SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_begin, "Opening app configuration file: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_end, "Finished app configuration")
SLIB_STATIC_STRING(_g_sdev_sapp_log_open_drawables_begin, "Opening drwable resources: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_open_resource_begin, "Opening resource file: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_begin, "Generating C++ files: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_target_path_is_empty, "App configuration: <generate-cpp>/<target-path> is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_target_path_invalid, "App configuration: Failed to create target path for generating cpp: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_namespace_is_empty, "App configuration: <generate-cpp>/<namespace> is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_generate_cpp_namespace_invalid, "App configuration: Invalid <generate-cpp>/<namespace>: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_resources_h_begin, "Generating: resources.h")

SLIB_STATIC_STRING(_g_sdev_sapp_error_configuration_value_empty, "App configuration: <%s> tag is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_configuration_value_invalid, "App configuration: value of <%s> tag is invalid: %s")

SLIB_STATIC_STRING(_g_sdev_sapp_error_out_of_memory, "Out of memory")
SLIB_STATIC_STRING(_g_sdev_sapp_error_file_not_found, "File not found: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_file_write_failed, "Failed to write to file: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_invalid_root_tag, "<sapp> tag is not declared for root element")
SLIB_STATIC_STRING(_g_sdev_sapp_error_configuration_tag_not_found, "<configuration> tag is not found in root element")
SLIB_STATIC_STRING(_g_sdev_sapp_error_invalid_tag, "Invalid tag: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_string_not_found, "String resource is not defined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_drawable_not_found, "Drawable resource is not defined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_drawable_not_image, "Drawable resource is not image: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_load_image_failed, "Failed to load image drawable resource: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_menu_not_found, "Menu resource is not defined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_layout_style_not_found, "layout style is not found: %s")

SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_raw_name_duplicated, "Raw Resource: Generated name %s is duplicated: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_raw_size_big, "Raw Resource: Size is larger than 16MB: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_raws_begin, "Generating: raws, raws.cpp")

SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_drawable_locale_invalid, "Drawable Resource: Invalid locale is found in the drawable directory name: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_drawable_filename_invalid, "Drawable Resource: Invalid image filename: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_drawable_not_defined_default, "Drawable Resource: default resource is not defined for this name: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_drawable_type_duplicated, "Drawable Resource: Invalid type is already defined on the name: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_drawable_load_image_failed, "Drawable Resource: Loading image file failed: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepieces_name_is_empty, "Nine-Pieces Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepieces_name_invalid, "Nine-Pieces Resource: Invalid name attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepieces_name_redefined, "Nine-Pieces Resource: name is redefined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepieces_attribute_invalid, "Nine-Pieces Resource: Invalid %s attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepatch_name_is_empty, "Nine-Patch Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepatch_name_invalid, "Nine-Patch Resource: Invalid name attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepatch_name_redefined, "Nine-Patch Resource: name is redefined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_ninepatch_attribute_invalid, "Nine-Patch Resource: Invalid %s attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_drawables_begin, "Generating: drawables.h, drawables.cpp")

SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_locale_invalid, "String Resource: Invalid locale attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_name_is_empty, "String Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_name_invalid, "String Resource: Invalid name attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_redefine_default, "String Resource: default value is redefined")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_string_redefine_locale, "String Resource: value is redefined for %s locale")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_strings_begin, "Generating: strings.h, strings.cpp")

SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_name_is_empty, "Menu Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_name_is_root, "Menu Resource: 'root' is not allowed for item name")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_name_invalid, "Menu Resource: Invalid name attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_name_redefined, "Menu Resource: name is redefined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_title_refer_invalid, "Menu Resource: title should be valid string value or string resource: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_icon_invalid, "Menu Resource: Invalid icon image resource: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_platform_invalid, "Menu Resource: Invalid platform attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_shortcutKey_invalid, "Menu Resource: Invalid shortcutKey attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_macShortcutKey_invalid, "Menu Resource: Invalid macShortcutKey attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_children_tag_invalid, "Menu Resource: Invalid tag: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_menu_item_name_redefined, "Menu Resource: item name is redefined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_menus_begin, "Generating: menus.h, menus.cpp")

SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_layout_type_invalid, "Layout Resource: Invalid layout type: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_layout_name_is_empty, "Layout Resource: name attribute is empty")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_layout_name_invalid, "Layout Resource: Invalid name attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_layout_name_redefined, "Layout Resource: name is redefined: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_layout_attribute_invalid, "Layout Resource: Invalid %s attribute value: %s")
SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_layout_scrollview_must_contain_one_child, "Layout Resource: ScrollView can contain only one child")
SLIB_STATIC_STRING(_g_sdev_sapp_log_generate_cpp_layouts_begin, "Generating: layouts.h, layouts.cpp")

#define RAW_MAX_SIZE 0x1000000 // 16MB

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
	
	{
		ListItems<String> list(File::getFiles(m_pathApp));
		for (sl_size i = 0; i < list.count; i++) {
			const String& fileName = list[i];
			if (fileName.isNotNull()) {
				if (fileName == "image") {
					if (!(_registerImageResources("image", m_pathApp + "/image", Locale::Unknown))) {
						return sl_false;
					}
				} else if (fileName.startsWith("image-")) {
					String strLocale = fileName.substring(9);
					Locale locale = Locale::Unknown;
					if (strLocale.isNotEmpty()) {
						locale = Locale(strLocale);
					}
					if (locale == Locale::Unknown || locale.isInvalid()) {
						_logError(_g_sdev_sapp_error_resource_drawable_locale_invalid.arg(fileName));
						return sl_false;
					}
					if (!(_registerImageResources(fileName, m_pathApp + "/" + fileName, locale))) {
						return sl_false;
					}
				}
			}
		}
	}
	
	const char* arrDirs[] = {
		"global",
		"ui"
	};
	{
		for (int i = 0; i < sizeof(arrDirs)/sizeof(const char*); i++) {
			String dirName = arrDirs[i];
			String pathDir = m_pathApp + "/" + dirName;
			ListItems<String> list(File::getFiles(pathDir));
			for (sl_size k = 0; k < list.count; k++) {
				const String& fileName = list[k];
				if (fileName.isNotNull()) {
					String path = pathDir + "/" + fileName;
					if (File::exists(path)) {
						if (!(File::isDirectory(path))) {
							if (File::getFileExtension(fileName) == "xml") {
								if (!(_parseResourcesXml(path))) {
									return sl_false;
								}
							}
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
	
	if (!_generateResourcesH(path)) {
		return sl_false;
	}
	if (!_generateStringsCpp(path)) {
		return sl_false;
	}
	if (!_generateRawsCpp(path)) {
		return sl_false;
	}
	if (!_generateDrawablesCpp(path)) {
		return sl_false;
	}
	if (!_generateMenusCpp(path)) {
		return sl_false;
	}
	if (!_generateLayoutsCpp(path)) {
		return sl_false;
	}

	return sl_true;
}

List< Ref<SAppLayoutResource> > SAppDocument::getLayouts()
{
	ObjectLocker lock(this);
	
	if (!m_flagOpened) {
		return List< Ref<SAppLayoutResource> >::null();
	}
	
	return m_layouts.values();
	
}

void SAppDocument::simulateLayoutInWindow(const String& layoutName)
{
	ObjectLocker lock(this);
	
	if (!m_flagOpened) {
		return;
	}

	Ref<SAppLayoutResource> layout = m_layouts.getValue(layoutName, Ref<SAppLayoutResource>::null());
	if (layout.isNotNull()) {
		_simulateLayoutInWindow(layout.ptr);
	}
	
}

/***************************************************
					Log
***************************************************/

void SAppDocument::_log(const String& text)
{
	SLIB_LOG(TAG, text);
}

void SAppDocument::_logError(const String& text)
{
	SLIB_LOG_ERROR(TAG, text);
}

void SAppDocument::_logError(const String& filePath, sl_size line, sl_size col, const String& text)
{
	SLIB_LOG_ERROR(TAG, String::format("%s(%d:%d)%n%s", filePath, line, col, text));
}

void SAppDocument::_logError(const Ref<XmlElement>& element, const String& text)
{
	if (element.isNotNull()) {
		SLIB_LOG_ERROR(TAG, String::format("%s(%d:%d)%n%s", element->getSourceFilePath(), element->getLineNumberInSource(), element->getColumnNumberInSource(), text));
	} else {
		SLIB_LOG_ERROR(TAG, text);
	}
}


/***************************************************
				Utilities
***************************************************/

String SAppDocument::_getShortcutKeyDefinitionString(const KeycodeAndModifiers& km, sl_bool flagMac)
{
	if (km.getKeycode() == Keycode::Unknown) {
		return "slib::Keycode::Unknown";
	}
	String ret;
	if (km.isControlKey()) {
		ret = "slib::Modifiers::Control";
	}
	if (flagMac) {
		if (km.isOptionKey()) {
			if (ret.isNotEmpty()) {
				ret += " | ";
			}
			ret = "slib::Modifiers::Option";
		}
	} else {
		if (km.isAltKey()) {
			if (ret.isNotEmpty()) {
				ret += " | ";
			}
			ret = "slib::Modifiers::Alt";
		}
	}
	if (km.isShiftKey()) {
		if (ret.isNotEmpty()) {
			ret += " | ";
		}
		ret = "slib::Modifiers::Shift";
	}
	if (flagMac) {
		if (km.isCommandKey()) {
			if (ret.isNotEmpty()) {
				ret += " | ";
			}
			ret = "slib::Modifiers::Command";
		}
	} else {
		if (km.isWindowsKey()) {
			if (ret.isNotEmpty()) {
				ret += " | ";
			}
			ret = "slib::Modifiers::Windows";
		}
	}
	if (ret.isNotEmpty()) {
		ret += " | ";
	}
	ret += "slib::Keycode::";
	ret += UI::getKeyName(km.getKeycode(), sl_false);
	return ret;
}


/***************************************************
				Resources Entry
***************************************************/

sl_bool SAppDocument::_parseConfiguration(const String& filePath, SAppConfiguration& conf)
{
	_log(_g_sdev_sapp_log_appconf_begin.arg(filePath));
	
	if (!(File::exists(filePath))) {
		_logError(_g_sdev_sapp_error_file_not_found.arg(filePath));
		return sl_false;
	}
	
	XmlParseParam param;
	param.setCreatingOnlyElementsAndTexts();
	Ref<XmlDocument> xml = Xml::parseXmlFromTextFile(filePath, param);
	if (param.flagError) {
		_logError(filePath, param.errorLine, param.errorColumn, param.errorMessage);
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
	
	root = root->getFirstChildElement("configuration");
	if (root.isNull()) {
		_logError(root, _g_sdev_sapp_error_configuration_tag_not_found);
		return sl_false;
	}
	
	// generate-cpp
	Ref<XmlElement> el_generate_cpp = root->getFirstChildElement("generate-cpp");
	if (el_generate_cpp.isNotNull()) {
		Ref<XmlElement> el_target_path = el_generate_cpp->getFirstChildElement("target-path");
		if (el_target_path.isNotNull()) {
			String strPath = el_target_path->getText();
			if (strPath.isEmpty()) {
				_logError(el_target_path, _g_sdev_sapp_error_configuration_value_empty.arg("target-path"));
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
				_logError(el_namespace, _g_sdev_sapp_error_configuration_value_invalid.arg("namespace", conf.generate_cpp_namespace));
				return sl_false;
			}
		}
		Ref<XmlElement> el_layout = el_generate_cpp->getFirstChildElement("layout");
		if (el_layout.isNotNull()) {
			ListLocker< Ref<XmlElement> > children(el_layout->getChildElements());
			for (sl_size i = 0; i < children.count; i++) {
				Ref<XmlElement> child = children[i];
				if (child.isNotNull() && child->getName() == "include-header") {
					String str = child->getText().trim();
					if (str.isNotEmpty()) {
						conf.generate_cpp_layout_include_headers.add(str);
					}
				}
			}
		}
	}
	
	_log(_g_sdev_sapp_log_appconf_end);
	
	return sl_true;
}

void SAppDocument::_freeResources()
{
	m_drawables.removeAll();
	m_strings.removeAll();
	m_menus.removeAll();
	m_raws.removeAll();
	m_layouts.removeAll();
	m_layoutStyles.removeAll();
	
	{
		ListLocker< Ref<SAppLayoutSimulationWindow> > windows(m_layoutSimulationWindows);
		for (sl_size i = 0; i < windows.count; i++) {
			Ref<SAppLayoutSimulationWindow> window = windows[i];
			if (window->isOpened()) {
				window->close();
			}
		}
	}
	m_layoutSimulationWindows.removeAll();
}

sl_bool SAppDocument::_parseResourcesXml(const String& filePath)
{
	_log(_g_sdev_sapp_log_open_resource_begin.arg(filePath));
	
	XmlParseParam param;
	param.setCreatingOnlyElementsAndTexts();
	Ref<XmlDocument> xml = Xml::parseXmlFromTextFile(filePath, param);
	if (param.flagError) {
		_logError(filePath, param.errorLine, param.errorColumn, param.errorMessage);
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
	for (sl_size i = 0; i < children.count; i++) {
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
			} else if (child->getName() == "nine-pieces") {
				if (!_parseNinePiecesDrawableResource(child)) {
					return sl_false;
				}
			} else if (child->getName() == "nine-patch") {
				if (!_parseNinePatchDrawableResource(child)) {
					return sl_false;
				}
			} else if (child->getName() == "menu") {
				if (!_parseMenuResource(child)) {
					return sl_false;
				}
			} else if (child->getName() == "layout-style") {
				if (!_parseLayoutStyle(child)) {
					return sl_false;
				}
			} else if (child->getName() == "layout") {
				if (!_parseLayoutResource(child)) {
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

sl_bool SAppDocument::_generateResourcesH(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_resources_h_begin);
	
	String content = String::format(
									"#ifndef CHECKHEADER_RESOURCES_H_%s%n"
									"#define CHECKHEADER_RESOURCES_H_%s%n%n"
									"#include \"strings.h\"%n"
									"#include \"drawables.h\"%n"
									"#include \"menus.h\"%n"
									"#include \"raws.h\"%n"
									"#include \"layouts.h\"%n"
									"%n#endif%n"
									, m_conf.generate_cpp_namespace);
	
	String pathHeader = targetPath + "/resources.h";
	if (!(File::writeAllTextUTF8(pathHeader, content))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathHeader));
		return sl_false;
	}
	
	return sl_true;
}


/***************************************************
				Raw Resources
***************************************************/

sl_bool SAppDocument::_registerRawResource(const String& path, const String& filePath, String& outName)
{
	String name = Resources::makeResourceName(path);
	if (m_raws.contains(name)) {
		_logError(_g_sdev_sapp_error_resource_raw_name_duplicated.arg(name, path));
		return sl_false;
	}
	Ref<SAppRawResource> res = new SAppRawResource;
	if (res.isNull()) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	res->name = name;
	res->path = path;
	res->filePath = filePath;
	if (!(m_raws.put(name, res))) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	outName = name;
	return sl_true;
}

sl_bool SAppDocument::_generateRawsCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_raws_begin);
	
	StringBuffer sbHeader, sbCpp, sbMap;
	
	sbHeader.add(String::format(
								"#ifndef CHECKHEADER_RESOURCE_RAWS_%s%n"
								"#define CHECKHEADER_RESOURCE_RAWS_%s%n%n"
								"#include <slib/core/resource.h>%n%n"
								"namespace %s%n"
								"{%n\tnamespace raw%n\t{%n%n"
								, m_conf.generate_cpp_namespace));
	sbCpp.add(String::format(
							 "#include \"raws.h\"%n"
							 "namespace %s%n"
							 "{%n\tnamespace raw%n\t{%n%n"
							 , m_conf.generate_cpp_namespace));
	
	sbMap.add("\t\tSLIB_DEFINE_RAW_RESOURCE_MAP_BEGIN\r\n");
	
	
	StringBuffer sbData;
	
	Iterator< Pair<String, Ref<SAppRawResource> > > iterator = m_raws.iterator();
	Pair< String, Ref<SAppRawResource> > pair;
	
	while (iterator.next(&pair)) {
		if (pair.value.isNotNull()) {
			
			Ref<SAppRawResource> res = pair.value;
			
			sbHeader.add(String::format("\t\tSLIB_DECLARE_RAW_RESOURCE(%s)%n", res->name));
			
			if (File::getSize(res->filePath) > RAW_MAX_SIZE) {
				_logError(_g_sdev_sapp_error_resource_raw_size_big.arg(res->filePath));
				return sl_false;
			}
			
			Memory mem = File::readAllBytes(res->filePath, RAW_MAX_SIZE);
			
			sbCpp.add(String::format("\t\tSLIB_DEFINE_RAW_RESOURCE(%s, %d)%n", res->name, mem.getSize()));
			
			sbMap.add(String::format("\t\t\tSLIB_DEFINE_RESOURCE_MAP_ITEM(%s)%n", res->name));
			
			sbData.add(String::format("\t\tnamespace %s {%n\t\t\tconst sl_uint8 bytes[] = {%n", res->name, mem.getData()));
			sbData.add(SDevUtil::generateBytesArrayDefinition(mem.getData(), mem.getSize(), 16, 4));
			static sl_char8 strDataEnd[] = "};\r\n\t\t}\r\n";
			sbData.addStatic(strDataEnd, sizeof(strDataEnd) - 1);
			
		}
	}
	
	sbMap.add("\t\tSLIB_DEFINE_RAW_RESOURCE_MAP_END\r\n");
	
	sbHeader.add("\r\n\t\tSLIB_DECLARE_RAW_RESOURCE_MAP\r\n\r\n\t}\r\n}\r\n\r\n#endif");
	
	sbCpp.add("\r\n");
	sbCpp.link(sbMap);
	
	sbCpp.add("\r\n");
	sbCpp.link(sbData);
	
	sbCpp.add("\r\n\t}\r\n}\r\n");
	
	String pathHeader = targetPath + "/raws.h";
	String contentHeader = sbHeader.merge();
	if (!(File::writeAllTextUTF8(pathHeader, contentHeader))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathHeader));
		return sl_false;
	}
	
	String pathCpp = targetPath + "/raws.cpp";
	String contentCpp = sbCpp.merge();
	if (!(File::writeAllTextUTF8(pathCpp, contentCpp))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathCpp));
		return sl_false;
	}
	
	return sl_true;
}


/***************************************************
				Drawable Resources
***************************************************/

sl_bool SAppDocument::_generateDrawablesCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_drawables_begin);
	
	StringBuffer sbHeader, sbCpp, sbMap;
	
	sbHeader.add(String::format(
								"#ifndef CHECKHEADER_RESOURCE_DRAWABLES_%s%n"
								"#define CHECKHEADER_RESOURCE_DRAWABLES_%s%n%n"
								"#include <slib/graphics/resource.h>%n%n"
								"namespace %s%n"
								"{%n\tnamespace drawable%n\t{%n%n"
								, m_conf.generate_cpp_namespace));
	sbCpp.add(String::format(
							 "#include \"drawables.h\"%n"
							 "#include \"raws.h\"%n%n"
							 "namespace %s%n"
							 "{%n\tnamespace drawable%n\t{%n%n"
							 , m_conf.generate_cpp_namespace));
	
	sbMap.add("\t\tSLIB_DEFINE_DRAWABLE_RESOURCE_MAP_BEGIN\r\n");
	
	// iterate image resources
	{
		Iterator< Pair<String, Ref<SAppDrawableResource> > > iterator = m_drawables.iterator();
		Pair< String, Ref<SAppDrawableResource> > pair;
		
		while (iterator.next(&pair)) {
			if (pair.value.isNotNull()) {
				Ref<SAppDrawableResource>& res = pair.value;
				if (res->type == SAppDrawableResource::typeImage) {
					_generateDrawablesCpp_Image(res.ptr, sbHeader, sbCpp, sbMap);
				}
			}
		}
	}
	
	// iterate other resources
	{
		Iterator< Pair<String, Ref<SAppDrawableResource> > > iterator = m_drawables.iterator();
		Pair< String, Ref<SAppDrawableResource> > pair;
		
		while (iterator.next(&pair)) {
			if (pair.value.isNotNull()) {
				Ref<SAppDrawableResource>& res = pair.value;
				if (res->type == SAppDrawableResource::typeNinePieces) {
					_generateDrawablesCpp_NinePieces(res.ptr, sbHeader, sbCpp, sbMap);
				} else if (res->type == SAppDrawableResource::typeNinePatch) {
					_generateDrawablesCpp_NinePatch(res.ptr, sbHeader, sbCpp, sbMap);
				}
			}
		}
	}
	
	sbMap.add("\t\tSLIB_DEFINE_DRAWABLE_RESOURCE_MAP_END\r\n");
	
	sbHeader.add("\r\n\t\tSLIB_DECLARE_DRAWABLE_RESOURCE_MAP\r\n\r\n\t}\r\n}\r\n\r\n#endif");
	
	sbCpp.link(sbMap);
	sbCpp.add("\r\n\t}\r\n}\r\n");
	
	
	String pathHeader = targetPath + "/drawables.h";
	String contentHeader = sbHeader.merge();
	if (!(File::writeAllTextUTF8(pathHeader, contentHeader))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathHeader));
		return sl_false;
	}
	
	String pathCpp = targetPath + "/drawables.cpp";
	String contentCpp = sbCpp.merge();
	if (!(File::writeAllTextUTF8(pathCpp, contentCpp))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathCpp));
		return sl_false;
	}
	
	return sl_true;
}

Ref<Drawable> SAppDocument::_getDrawableValue(SAppDrawableValue& value)
{
	if (!(value.flagDefined)) {
		return Ref<Drawable>::null();
	}
	if (value.flagNull) {
		return Ref<Drawable>::null();
	}
	Ref<SAppDrawableResource> res = m_drawables.getValue(value.resourceName, Ref<SAppDrawableResource>::null());
	if (res.isNull()) {
		_logError(_g_sdev_sapp_error_drawable_not_found.arg(value.resourceName));
		return Ref<Drawable>::null();
	}
	
	Ref<Drawable> drawable;
	if (res->type == SAppDrawableResource::typeImage) {
		drawable = _getDrawableValue_Image(res.ptr);
	} else if (res->type == SAppDrawableResource::typeNinePieces) {
		drawable = _getDrawableValue_NinePieces(res.ptr);
	} else if (res->type == SAppDrawableResource::typeNinePatch) {
		drawable = _getDrawableValue_NinePatch(res.ptr);
	}
	if (drawable.isNotNull()) {
		if (!(value.flagWhole)) {
			drawable = Drawable::createSubDrawable(drawable, value.x, value.y, value.width, value.height);
		}
		if (value.func == SAppDrawableValue::FUNC_NINEPATCH) {
			drawable = NinePatchDrawable::create(_getDimensionValue(value.ninePatch_leftWidthDst), _getDimensionValue(value.ninePatch_rightWidthDst), _getDimensionValue(value.ninePatch_topHeightDst), _getDimensionValue(value.ninePatch_bottomHeightDst), drawable, value.ninePatch_leftWidth, value.ninePatch_rightWidth, value.ninePatch_topHeight, value.ninePatch_bottomHeight);
		}
	}
	return drawable;
	
}

Ref<Image> SAppDocument::_getImageValue(SAppDrawableValue& value)
{
	if (!(value.flagDefined) || value.flagNull || !(value.flagWhole) || value.func != SAppDrawableValue::FUNC_NONE) {
		return Ref<Image>::null();
	}
	Ref<Drawable> drawable = _getDrawableValue(value);
	if (Image::checkInstance(drawable.ptr)) {
		return Ref<Image>::from(drawable);
	} else if (MipmapDrawable::checkInstance(drawable.ptr)) {
		MipmapDrawable* mipmap = (MipmapDrawable*)(drawable.ptr);
		drawable = mipmap->getSource(0);
		if (Image::checkInstance(drawable.ptr)) {
			return Ref<Image>::from(drawable);
		}
	}
	return Ref<Image>::null();
}

sl_bool SAppDocument::_registerImageResources(const String& resourcePath, const String& fileDirPath, const Locale& locale)
{
	_log(_g_sdev_sapp_log_open_drawables_begin.arg(fileDirPath));
	List<String> _list = File::getFiles(fileDirPath);
	_list.sort();
	ListItems<String> list(_list);
	for (sl_size i = 0; i < list.count; i++) {
		const String& fileName = list[i];
		String ext = File::getFileExtension(fileName);
		if (ext == "png" || ext == "jpg" || ext == "jpeg") {
			String name = File::getFileNameOnly(fileName);
			sl_reg indexSharp = name.indexOf('#');
			sl_bool flagMain = sl_true;
			if (indexSharp >= 0) {
				flagMain = sl_false;
				name = name.substring(0, indexSharp);
			}
			if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
				_logError(_g_sdev_sapp_error_resource_drawable_filename_invalid.arg(resourcePath + "/" + fileName));
				return sl_false;
			}
			Ref<SAppDrawableResource> res = m_drawables.getValue(name, Ref<SAppDrawableResource>::null());
			if (res.isNull()) {
				if (locale != Locale::Unknown) {
					_logError(_g_sdev_sapp_error_resource_drawable_not_defined_default.arg(name));
					return sl_false;
				}
				res = new SAppDrawableResource;
				if (res.isNull()) {
					_logError(_g_sdev_sapp_error_out_of_memory);
					return sl_false;
				}
				res->name = name;
				res->type = SAppDrawableResource::typeImage;
				res->imageAttrs = new SAppDrawableResourceImageAttributes;
				if (res->imageAttrs.isNull()) {
					_logError(_g_sdev_sapp_error_out_of_memory);
					return sl_false;
				}
				if (!(m_drawables.put(name, res))) {
					_logError(_g_sdev_sapp_error_out_of_memory);
					return sl_false;
				}
			}
			if (res->type != SAppDrawableResource::typeImage) {
				_logError(_g_sdev_sapp_error_resource_drawable_type_duplicated.arg(resourcePath + "/" + fileName));
				return sl_false;
			}
			
			SAppDrawableResourceImageAttributes* imageAttr = res->imageAttrs.ptr;
			
			List< Ref<SAppDrawableResourceImageItem> > list;
			if (locale == Locale::Unknown) {
				list = imageAttr->defaultImages;
				if (list.isNull()) {
					_logError(_g_sdev_sapp_error_out_of_memory);
					return sl_false;
				}
			} else {
				imageAttr->images.get(locale, &list);
				if (list.isNull()) {
					list = List< Ref<SAppDrawableResourceImageItem> >::create();
					if (list.isNull()) {
						_logError(_g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
					if (!(imageAttr->images.put(locale, list))) {
						_logError(_g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
			
			Ref<SAppDrawableResourceImageItem> item = new SAppDrawableResourceImageItem;
			if (item.isNull()) {
				_logError(_g_sdev_sapp_error_out_of_memory);
				return sl_false;
			}
			item->fileName = fileName;
			item->filePath = fileDirPath + "/" + fileName;
			if (!(_registerRawResource(resourcePath + "/" + fileName, item->filePath, item->rawName))) {
				return sl_false;
			}
			if (flagMain) {
				if (!(list.insert(0, item))) {
					_logError(_g_sdev_sapp_error_out_of_memory);
					return sl_false;
				}
			} else {
				if (!(list.add(item))) {
					_logError(_g_sdev_sapp_error_out_of_memory);
					return sl_false;
				}
			}
		}
	}
	return sl_true;
}

sl_bool SAppDocument::_generateDrawablesCpp_Image(SAppDrawableResource* res, StringBuffer& sbHeader, StringBuffer& sbCpp, StringBuffer& sbMap)
{
	SAppDrawableResourceImageAttributes* imageAttr = res->imageAttrs.ptr;
	
	Ref<SAppDrawableResourceImageItem> item;
	
	sl_bool flagSuccess = sl_false;
	
	sl_uint32 defaultWidth = 0;
	sl_uint32 defaultHeight = 0;
	
	if (imageAttr->defaultImages.getItem(0, &item)) {
		
		Ref<Image> image = item->loadImage();
		
		if (image.isNotNull()) {
			defaultWidth = image->getWidth();
			defaultHeight = image->getHeight();
			flagSuccess = sl_true;
		}
		
	}
	
	if (!flagSuccess) {
		_logError(_g_sdev_sapp_error_load_image_failed.arg(res->name));
		return sl_false;
	}
	
	
	sbHeader.add(String::format("\t\tSLIB_DECLARE_IMAGE_RESOURCE(%s)%n", res->name));
	
	if (imageAttr->images.isEmpty() && imageAttr->defaultImages.getCount() == 1 && item.isNotNull()) {
		
		sbCpp.add(String::format("\t\tSLIB_DEFINE_IMAGE_RESOURCE_SIMPLE(%s, %d, %d, raw::%s::size, raw::%s::bytes)%n%n", res->name, defaultWidth, defaultHeight, item->rawName));
		
	} else {
		
		sbCpp.add(String::format("\t\tSLIB_DEFINE_IMAGE_RESOURCE_BEGIN(%s, %d, %d)%n", res->name, defaultWidth, defaultHeight));
		
		CList< Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > > listPairs;
		// locales
		{
			Iterator< Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > > iteratorItems = imageAttr->images.iterator();
			Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > pairItems;
			while (iteratorItems.next(&pairItems)) {
				if (pairItems.key.getCountry() != Country::Unknown) {
					if (!(listPairs.add_NoLock(pairItems))) {
						_logError(_g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		}
		// languages
		{
			Iterator< Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > > iteratorItems = imageAttr->images.iterator();
			Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > pairItems;
			while (iteratorItems.next(&pairItems)) {
				if (pairItems.key.getCountry() == Country::Unknown) {
					if (!(listPairs.add_NoLock(pairItems))) {
						_logError(_g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		}
		// default
		{
			Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > pairItems;
			pairItems.key = Locale::Unknown;
			pairItems.value = imageAttr->defaultImages;
			if (!(listPairs.add_NoLock(pairItems))) {
				_logError(_g_sdev_sapp_error_out_of_memory);
				return sl_false;
			}
		}
		
		ListItems< Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > > pairs(listPairs);
		for (sl_size iPair = 0; iPair < pairs.count; iPair++) {
			
			String strLocale = pairs[iPair].key.toString();
			
			ListLocker< Ref<SAppDrawableResourceImageItem> > items(pairs[iPair].value);
			
			if (pairs[iPair].key == Locale::Unknown) {
				static sl_char8 str[] = "\t\t\tSLIB_DEFINE_IMAGE_RESOURCE_DEFAULT_LIST_BEGIN\r\n";
				sbCpp.addStatic(str, sizeof(str)-1);
			} else {
				sbCpp.add(String::format("\t\t\tSLIB_DEFINE_IMAGE_RESOURCE_LIST_BEGIN(%s)%n", strLocale));
			}
			
			for (sl_size i = 0; i < items.count; i++) {
				
				item = items[i];
				
				Ref<Image> image = item->loadImage();
				if (image.isNull()) {
					_logError(_g_sdev_sapp_error_resource_drawable_load_image_failed.arg(item->filePath));
					return sl_false;
				}
				sbCpp.add(String::format("\t\t\t\tSLIB_DEFINE_IMAGE_RESOURCE_ITEM(%d, %d, raw::%s::size, raw::%s::bytes)%n", image->getWidth(), image->getHeight(), item->rawName));
			}
			
			static sl_char8 strEnd[] = "\t\t\tSLIB_DEFINE_IMAGE_RESOURCE_LIST_END\r\n";
			sbCpp.addStatic(strEnd, sizeof(strEnd)-1);
			
			
		}
		
		static sl_char8 strEnd[] = "\t\tSLIB_DEFINE_IMAGE_RESOURCE_END\r\n\r\n";
		sbCpp.addStatic(strEnd, sizeof(strEnd)-1);
		
	}
	
	sbMap.add(String::format("\t\t\tSLIB_DEFINE_RESOURCE_MAP_ITEM(%s)%n", res->name));
	
	return sl_true;
}

Ref<Drawable> SAppDocument::_getDrawableValue_Image(SAppDrawableResource* res)
{
	SAppDrawableResourceImageAttributes* imageAttr = res->imageAttrs.ptr;
	
	Ref<SAppDrawableResourceImageItem> item;

	sl_size n = imageAttr->defaultImages.getCount();
	if (n == 1) {
		if (imageAttr->defaultImages.getItem(0, &item)) {
			if (item.isNotNull()) {
				Ref<Image> image = item->loadImage();
				if (image.isNotNull()) {
					return image;
				} else {
					_logError(_g_sdev_sapp_error_load_image_failed.arg(res->name));
					return Ref<Drawable>::null();
				}
			}
		}
	} else {
		Ref<MipmapDrawable> mipmap = new MipmapDrawable;
		if (mipmap.isNotNull()) {
			sl_real defaultWidth = 1;
			sl_real defaultHeight = 1;
			ListLocker< Ref<SAppDrawableResourceImageItem> > items(imageAttr->defaultImages);
			for (sl_size i = 0; i < items.count; i++) {
				item = items[i];
				if (item.isNotNull()) {
					Ref<Image> image = item->loadImage();
					if (image.isNotNull()) {
						sl_real width = (sl_real)(image->getWidth());
						sl_real height = (sl_real)(image->getHeight());
						mipmap->addSource(image, width, height);
						if (width > SLIB_EPSILON && height > SLIB_EPSILON) {
							defaultWidth = width;
							defaultHeight = height;
						}
					} else {
						_logError(_g_sdev_sapp_error_resource_drawable_load_image_failed.arg(item->filePath));
						return Ref<Drawable>::null();
					}
				}
			}
			mipmap->setDrawableWidth(defaultWidth);
			mipmap->setDrawableHeight(defaultHeight);
			return mipmap;
		}
	}
	return Ref<Drawable>::null();
}

#define LOG_ERROR_NINEPIECES_ATTR(NAME) \
	_logError(element, _g_sdev_sapp_error_resource_ninepieces_attribute_invalid.arg(#NAME, str_##NAME));

#define PARSE_AND_CHECK_NINEPIECES_ATTR(ATTR, NAME) \
	String str_##NAME = element->getAttribute(#NAME); \
	if (!(ATTR NAME.parse(str_##NAME))) { \
		LOG_ERROR_NINEPIECES_ATTR(NAME) \
		return sl_false; \
	}

sl_bool SAppDocument::_parseNinePiecesDrawableResource(const Ref<XmlElement>& element)
{
	if (element.isNull()) {
		return sl_false;
	}
	
	String name = element->getAttribute("name");
	if (name.isEmpty()) {
		_logError(element, _g_sdev_sapp_error_resource_ninepieces_name_is_empty);
		return sl_false;
	}
	if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
		_logError(element, _g_sdev_sapp_error_resource_ninepieces_name_invalid.arg(name));
		return sl_false;
	}

	if (m_drawables.contains(name)) {
		_logError(element, _g_sdev_sapp_error_resource_ninepieces_name_redefined.arg(name));
		return sl_false;
	}
	
	Ref<SAppDrawableResource> res = new SAppDrawableResource;
	if (res.isNull()) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	res->name = name;
	res->type = SAppDrawableResource::typeNinePieces;
	res->ninePiecesAttrs = new SAppDrawableResourceNinePiecesAttributes;
	if (res->ninePiecesAttrs.isNull()) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	SAppDrawableResourceNinePiecesAttributes* attr = res->ninePiecesAttrs.ptr;

	attr->element = element;
	
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, leftWidth)
	if (!(attr->leftWidth.checkGlobal())) {
		LOG_ERROR_NINEPIECES_ATTR(leftWidth)
		return sl_false;
	}
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, rightWidth)
	if (!(attr->rightWidth.checkGlobal())) {
		LOG_ERROR_NINEPIECES_ATTR(rightWidth)
		return sl_false;
	}
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, topHeight)
	if (!(attr->topHeight.checkGlobal())) {
		LOG_ERROR_NINEPIECES_ATTR(topHeight)
		return sl_false;
	}
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, bottomHeight)
	if (!(attr->bottomHeight.checkGlobal())) {
		LOG_ERROR_NINEPIECES_ATTR(bottomHeight)
		return sl_false;
	}
	
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, topLeft)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, top)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, topRight)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, left)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, center)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, right)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, bottomLeft)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, bottom)
	PARSE_AND_CHECK_NINEPIECES_ATTR(attr->, bottomRight)

	if (!(m_drawables.put(name, res))) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateDrawablesCpp_NinePieces(SAppDrawableResource* res, StringBuffer& sbHeader, StringBuffer& sbCpp, StringBuffer& sbMap)
{
	SAppDrawableResourceNinePiecesAttributes* attr = res->ninePiecesAttrs.ptr;
	
	sbHeader.add(String::format("\t\tSLIB_DECLARE_NINEPIECES_RESOURCE(%s)%n", res->name));
	
	if (!(_checkDrawableValueAvailable(attr->topLeft, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->top, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->topRight, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->left, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->center, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->right, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->bottomLeft, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->bottom, attr->element))) {
		return sl_false;
	}
	if (!(_checkDrawableValueAvailable(attr->bottomRight, attr->element))) {
		return sl_false;
	}
	
	sbCpp.add(String::format("\t\tSLIB_DEFINE_NINEPIECES_RESOURCE(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)%n%n", res->name, attr->leftWidth.getAccessString(), attr->rightWidth.getAccessString(), attr->topHeight.getAccessString(), attr->bottomHeight.getAccessString(), attr->topLeft.getAccessString(), attr->top.getAccessString(), attr->topRight.getAccessString(), attr->left.getAccessString(), attr->center.getAccessString(), attr->right.getAccessString(), attr->bottomLeft.getAccessString(), attr->bottom.getAccessString(), attr->bottomRight.getAccessString()));
	
	sbMap.add(String::format("\t\t\tSLIB_DEFINE_RESOURCE_MAP_ITEM(%s)%n", res->name));
	
	return sl_true;
}

Ref<Drawable> SAppDocument::_getDrawableValue_NinePieces(SAppDrawableResource* res)
{
	SAppDrawableResourceNinePiecesAttributes* attr = res->ninePiecesAttrs.ptr;
	
	do {
		if (!(_checkDrawableValueAvailable(attr->topLeft, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->top, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->topRight, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->left, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->center, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->right, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->bottomLeft, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->bottom, attr->element))) {
			break;
		}
		if (!(_checkDrawableValueAvailable(attr->bottomRight, attr->element))) {
			break;
		}
		
		return NinePiecesDrawable::create(_getDimensionValue(attr->leftWidth), _getDimensionValue(attr->rightWidth), _getDimensionValue(attr->topHeight), _getDimensionValue(attr->bottomHeight), _getDrawableValue(attr->topLeft), _getDrawableValue(attr->top), _getDrawableValue(attr->topRight), _getDrawableValue(attr->left), _getDrawableValue(attr->center), _getDrawableValue(attr->right), _getDrawableValue(attr->bottomLeft), _getDrawableValue(attr->bottom), _getDrawableValue(attr->bottomRight));
		
	} while (0);
	
	return Ref<Drawable>::null();
}

#define LOG_ERROR_NINEPATCH_ATTR(NAME) \
	_logError(element, _g_sdev_sapp_error_resource_ninepatch_attribute_invalid.arg(#NAME, str_##NAME));

#define PARSE_AND_CHECK_NINEPATCH_ATTR(ATTR, NAME) \
	String str_##NAME = element->getAttribute(#NAME); \
	if (!(ATTR NAME.parse(str_##NAME))) { \
		LOG_ERROR_NINEPATCH_ATTR(NAME) \
		return sl_false; \
	}

sl_bool SAppDocument::_parseNinePatchDrawableResource(const Ref<XmlElement>& element)
{
	if (element.isNull()) {
		return sl_false;
	}
	
	String name = element->getAttribute("name");
	if (name.isEmpty()) {
		_logError(element, _g_sdev_sapp_error_resource_ninepatch_name_is_empty);
		return sl_false;
	}
	if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
		_logError(element, _g_sdev_sapp_error_resource_ninepatch_name_invalid.arg(name));
		return sl_false;
	}
	
	if (m_drawables.contains(name)) {
		_logError(element, _g_sdev_sapp_error_resource_ninepatch_name_redefined.arg(name));
		return sl_false;
	}
	
	Ref<SAppDrawableResource> res = new SAppDrawableResource;
	if (res.isNull()) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	res->name = name;
	res->type = SAppDrawableResource::typeNinePatch;
	res->ninePatchAttrs = new SAppDrawableResourceNinePatchAttributes;
	if (res->ninePatchAttrs.isNull()) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	SAppDrawableResourceNinePatchAttributes* attr = res->ninePatchAttrs.ptr;
	
	attr->element = element;
	
	PARSE_AND_CHECK_NINEPATCH_ATTR(attr->, dstLeftWidth)
	if (!(attr->dstLeftWidth.checkGlobal())) {
		LOG_ERROR_NINEPATCH_ATTR(dstLeftWidth)
		return sl_false;
	}
	PARSE_AND_CHECK_NINEPATCH_ATTR(attr->, dstRightWidth)
	if (!(attr->dstRightWidth.checkGlobal())) {
		LOG_ERROR_NINEPATCH_ATTR(dstRightWidth)
		return sl_false;
	}
	PARSE_AND_CHECK_NINEPATCH_ATTR(attr->, dstTopHeight)
	if (!(attr->dstTopHeight.checkGlobal())) {
		LOG_ERROR_NINEPATCH_ATTR(dstTopHeight)
		return sl_false;
	}
	PARSE_AND_CHECK_NINEPATCH_ATTR(attr->, dstBottomHeight)
	if (!(attr->dstBottomHeight.checkGlobal())) {
		LOG_ERROR_NINEPATCH_ATTR(dstBottomHeight)
		return sl_false;
	}
	
	PARSE_AND_CHECK_NINEPATCH_ATTR(attr->, src)
	if (!(attr->src.flagDefined)) {
		LOG_ERROR_NINEPATCH_ATTR(src)
		return sl_false;
	}
	
	SAppFloatValue leftWidth;
	PARSE_AND_CHECK_NINEPATCH_ATTR(, leftWidth)
	attr->leftWidth = leftWidth.value;
	if (!(attr->dstLeftWidth.flagDefined)) {
		attr->dstLeftWidth.amount = leftWidth.value;
		attr->dstLeftWidth.unit = SAppDimensionValue::PX;
		attr->dstLeftWidth.flagDefined = sl_true;
	}
	
	SAppFloatValue rightWidth;
	PARSE_AND_CHECK_NINEPATCH_ATTR(, rightWidth)
	attr->rightWidth = rightWidth.value;
	if (!(attr->dstRightWidth.flagDefined)) {
		attr->dstRightWidth.amount = rightWidth.value;
		attr->dstRightWidth.unit = SAppDimensionValue::PX;
		attr->dstRightWidth.flagDefined = sl_true;
	}

	SAppFloatValue topHeight;
	PARSE_AND_CHECK_NINEPATCH_ATTR(, topHeight)
	attr->topHeight = topHeight.value;
	if (!(attr->dstTopHeight.flagDefined)) {
		attr->dstTopHeight.amount = topHeight.value;
		attr->dstTopHeight.unit = SAppDimensionValue::PX;
		attr->dstTopHeight.flagDefined = sl_true;
	}
	
	SAppFloatValue bottomHeight;
	PARSE_AND_CHECK_NINEPATCH_ATTR(, bottomHeight)
	attr->bottomHeight = bottomHeight.value;
	if (!(attr->dstBottomHeight.flagDefined)) {
		attr->dstBottomHeight.amount = bottomHeight.value;
		attr->dstBottomHeight.unit = SAppDimensionValue::PX;
		attr->dstBottomHeight.flagDefined = sl_true;
	}
	
	if (!(m_drawables.put(name, res))) {
		_logError(_g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateDrawablesCpp_NinePatch(SAppDrawableResource* res, StringBuffer& sbHeader, StringBuffer& sbCpp, StringBuffer& sbMap)
{
	SAppDrawableResourceNinePatchAttributes* attr = res->ninePatchAttrs.ptr;
	
	sbHeader.add(String::format("\t\tSLIB_DECLARE_NINEPATCH_RESOURCE(%s)%n", res->name));
	
	if (!(_checkDrawableValueAvailable(attr->src, attr->element))) {
		return sl_false;
	}
	
	sbCpp.add(String::format("\t\tSLIB_DEFINE_NINEPATCH_RESOURCE(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)%n%n", res->name, attr->dstLeftWidth.getAccessString(), attr->dstRightWidth.getAccessString(), attr->dstTopHeight.getAccessString(), attr->dstBottomHeight.getAccessString(), attr->src.getAccessString(), attr->leftWidth, attr->rightWidth, attr->topHeight, attr->bottomHeight));
	
	sbMap.add(String::format("\t\t\tSLIB_DEFINE_RESOURCE_MAP_ITEM(%s)%n", res->name));
	
	return sl_true;
}

Ref<Drawable> SAppDocument::_getDrawableValue_NinePatch(SAppDrawableResource* res)
{
	SAppDrawableResourceNinePatchAttributes* attr = res->ninePatchAttrs.ptr;
	if (!(_checkDrawableValueAvailable(attr->src, attr->element))) {
		return Ref<Drawable>::null();
	}
	return NinePatchDrawable::create(_getDimensionValue(attr->dstLeftWidth), _getDimensionValue(attr->dstRightWidth), _getDimensionValue(attr->dstTopHeight), _getDimensionValue(attr->dstBottomHeight), _getDrawableValue(attr->src), attr->leftWidth, attr->rightWidth, attr->topHeight, attr->bottomHeight);
}

sl_bool SAppDocument::_checkDrawableValueAvailable(SAppDrawableValue& value, const Ref<XmlElement>& elementError)
{
	if (!(value.flagDefined)) {
		return sl_true;
	}
	if (value.flagNull) {
		return sl_true;
	}
	if (m_drawables.contains(value.resourceName)) {
		return sl_true;
	} else {
		_logError(elementError, _g_sdev_sapp_error_drawable_not_found.arg(value.resourceName));
		return sl_false;
	}
}

sl_bool SAppDocument::_checkDrawableValueImage(SAppDrawableValue& value, const Ref<XmlElement>& elementError)
{
	if (!(value.flagDefined)) {
		return sl_true;
	}
	if (value.flagNull) {
		return sl_true;
	}
	if (!(value.flagWhole) || value.func != SAppDrawableValue::FUNC_NONE) {
		_logError(elementError, _g_sdev_sapp_error_drawable_not_image.arg(value.resourceName));
		return sl_false;
	}
	Ref<SAppDrawableResource> res = m_drawables.getValue(value.resourceName, Ref<SAppDrawableResource>::null());
	if (res.isNull()) {
		_logError(elementError, _g_sdev_sapp_error_drawable_not_found.arg(value.resourceName));
		return sl_false;
	}
	if (res->type != SAppDrawableResource::typeImage) {
		_logError(elementError, _g_sdev_sapp_error_drawable_not_image.arg(value.resourceName));
		return sl_false;
	}
	return sl_true;
}

/***************************************************
				String Resources
***************************************************/

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

sl_bool SAppDocument::_generateStringsCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_strings_begin);
	
	StringBuffer sbHeader, sbCpp, sbMap;
	
	sbHeader.add(String::format(
								"#ifndef CHECKHEADER_RESOURCE_STRINGS_%s%n"
								"#define CHECKHEADER_RESOURCE_STRINGS_%s%n%n"
								"#include <slib/core/resource.h>%n%n"
								"namespace %s%n"
								"{%n\tnamespace string%n\t{%n%n"
								, m_conf.generate_cpp_namespace));
	sbCpp.add(String::format(
							 "#include \"strings.h\"%n%n"
							 "namespace %s%n"
							 "{%n\tnamespace string%n\t{%n%n"
							 , m_conf.generate_cpp_namespace));
	
	sbMap.add("\t\tSLIB_DEFINE_STRING_RESOURCE_MAP_BEGIN\r\n");
	
	Iterator< Pair<String, Ref<SAppStringResource> > > iterator = m_strings.iterator();
	Pair< String, Ref<SAppStringResource> > pair;
	
	while (iterator.next(&pair)) {
		if (pair.value.isNotNull()) {
			
			sbHeader.add(String::format("\t\tSLIB_DECLARE_STRING_RESOURCE(%s)%n", pair.key));
			
			String defValue = pair.value->defaultValue;
			
			if (pair.value->values.isEmpty()) {
				sbCpp.add(String::format("\t\tSLIB_DEFINE_STRING_RESOURCE_SIMPLE(%s, \"%s\")%n%n", pair.key, defValue.applyBackslashEscapes(sl_true, sl_false, sl_true)));
			} else {
				
				sbCpp.add(String::format("\t\tSLIB_DEFINE_STRING_RESOURCE_BEGIN(%s, \"%s\")%n", pair.key, defValue.applyBackslashEscapes(sl_true, sl_false, sl_true)));
				
				// locales
				{
					Iterator< Pair<Locale, String> > iteratorValues = pair.value->values.iterator();
					Pair< Locale, String > pairValues;
					while (iteratorValues.next(&pairValues)) {
						if (pairValues.key.getCountry() != Country::Unknown) {
							sbCpp.add(String::format("\t\t\tSLIB_DEFINE_STRING_RESOURCE_VALUE(%s, \"%s\")%n", pairValues.key.toString(), pairValues.value.applyBackslashEscapes(sl_true, sl_false, sl_true)));
						}
					}
				}
				// languages
				{
					Iterator< Pair<Locale, String> > iteratorValues = pair.value->values.iterator();
					Pair< Locale, String > pairValues;
					while (iteratorValues.next(&pairValues)) {
						if (pairValues.key.getCountry() == Country::Unknown) {
							sbCpp.add(String::format("\t\t\tSLIB_DEFINE_STRING_RESOURCE_VALUE(%s, \"%s\")%n", pairValues.key.toString(), pairValues.value.applyBackslashEscapes(sl_true, sl_false, sl_true)));
						}
					}
				}
				
				static sl_char8 strEnd[] = "\t\tSLIB_DEFINE_STRING_RESOURCE_END\r\n\r\n";
				sbCpp.addStatic(strEnd, sizeof(strEnd)-1);
			}
			
			sbMap.add(String::format("\t\t\tSLIB_DEFINE_RESOURCE_MAP_ITEM(%s)%n", pair.key));
			
		}
	}
	
	sbMap.add("\t\tSLIB_DEFINE_STRING_RESOURCE_MAP_END\r\n");
	
	sbHeader.add("\r\n\t\tSLIB_DECLARE_STRING_RESOURCE_MAP\r\n\r\n\t}\r\n}\r\n\r\n#endif");
	
	sbCpp.link(sbMap);
	sbCpp.add("\r\n\t}\r\n}\r\n");
	
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

String SAppDocument::_getStringValue(const SAppStringValue& value)
{
	if (!(value.flagDefined)) {
		return String::null();
	}
	if (!(value.flagReferResource)) {
		return value.valueOrName;
	}
	Ref<SAppStringResource> res = m_strings.getValue(value.valueOrName, Ref<SAppStringResource>::null());
	if (res.isNull()) {
		_logError(_g_sdev_sapp_error_string_not_found.arg(value.valueOrName));
		return String::null();
	}
	return res->defaultValue;
}

sl_bool SAppDocument::_checkStringValueAvailable(SAppStringValue& value, const Ref<XmlElement>& elementError)
{
	if (!(value.flagDefined)) {
		return sl_true;
	}
	if (!(value.flagReferResource)) {
		return sl_true;
	}
	if (m_strings.contains(value.valueOrName)) {
		return sl_true;
	} else {
		_logError(elementError, _g_sdev_sapp_error_string_not_found.arg(value.valueOrName));
		return sl_false;
	}
}

/***************************************************
				Menu Resources
***************************************************/

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
	if (m_menus.contains(name)) {
		_logError(element, _g_sdev_sapp_error_resource_menu_name_redefined.arg(name));
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
		if (name == 'root') {
			_logError(element, _g_sdev_sapp_error_resource_menu_name_is_root);
			return Ref<SAppMenuResourceItem>::null();
		}
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
		if (title == "@") {
			item->title.flagDefined = sl_true;
			item->title.flagReferResource = sl_true;
			item->title.valueOrName = String::format("menu_%s_%s", menu->name, name);
		} else {
			if (!(item->title.parse(title))) {
				_logError(element, _g_sdev_sapp_error_resource_menu_title_refer_invalid.arg(title));
				return Ref<SAppMenuResourceItem>::null();
			}
		}
		
		String strIcon;
		strIcon = element->getAttribute("icon");
		if (!(item->icon.parseWhole(strIcon))) {
			_logError(element, _g_sdev_sapp_error_resource_menu_icon_invalid.arg(strIcon));
			return Ref<SAppMenuResourceItem>::null();
		}
		strIcon = element->getAttribute("checkedIcon");
		if (!(item->checkedIcon.parseWhole(strIcon))) {
			_logError(element, _g_sdev_sapp_error_resource_menu_icon_invalid.arg(strIcon));
			return Ref<SAppMenuResourceItem>::null();
		}
	}
	
	if (item->type == SAppMenuResourceItem::typeItem) {
		String strShortcutKey = element->getAttribute("shortcutKey");
		if (strShortcutKey.isEmpty()) {
			item->shortcutKey = 0;
		} else {
			if (!(item->shortcutKey.parse(strShortcutKey))) {
				_logError(element, _g_sdev_sapp_error_resource_menu_shortcutKey_invalid.arg(strShortcutKey));
				return Ref<SAppMenuResourceItem>::null();
			}
		}
		strShortcutKey = element->getAttribute("macShortcutKey");
		if (strShortcutKey.isNull()) {
			item->macShortcutKey = item->shortcutKey;
		} else if (strShortcutKey.isEmpty()) {
			item->macShortcutKey = 0;
		} else {
			if (!(item->macShortcutKey.parse(strShortcutKey))) {
				_logError(element, _g_sdev_sapp_error_resource_menu_macShortcutKey_invalid.arg(strShortcutKey));
				return Ref<SAppMenuResourceItem>::null();
			}
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

sl_bool SAppDocument::_generateMenusCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_menus_begin);
	
	StringBuffer sbHeader, sbCpp;
	sbHeader.add(String::format(
								"#ifndef CHECKHEADER_RESOURCE_MENUS_%s%n"
								"#define CHECKHEADER_RESOURCE_MENUS_%s%n%n"
								"#include <slib/ui/resource.h>%n%n"
								"namespace %s%n"
								"{%n\tnamespace menu%n\t{%n%n"
								, m_conf.generate_cpp_namespace));
	
	sbCpp.add(String::format(
							 "#include \"menus.h\"%n"
							 "#include \"strings.h\"%n"
							 "#include \"drawables.h\"%n%n"
							 "namespace %s%n"
							 "{%n\tnamespace menu%n\t{%n%n"
							 , m_conf.generate_cpp_namespace));
	
	
	Iterator< Pair<String, Ref<SAppMenuResource> > > iterator = m_menus.iterator();
	Pair< String, Ref<SAppMenuResource> > pair;
	
	while (iterator.next(&pair)) {
		if (pair.value.isNotNull()) {
			
			sbHeader.add(String::format("\t\tSLIB_DECLARE_MENU_BEGIN(%s)%n", pair.key));
			sbCpp.add(String::format("\t\tSLIB_DEFINE_MENU_BEGIN(%s)%n", pair.key));
			
			ListLocker< Ref<SAppMenuResourceItem> > items(pair.value->children);
			for (sl_size i = 0; i < items.count; i++) {
				Ref<SAppMenuResourceItem>& item = items[i];
				if (item.isNotNull()) {
					if (!_generateMenusCpp_Item("root", SAppMenuResourceItem::all_platforms, item.ptr, sbHeader, sbCpp, 3)) {
						return sl_false;
					}
				}
			}
			
			static sl_char8 strEndHeader[] = "\t\tSLIB_DECLARE_MENU_END\r\n\r\n";
			sbHeader.addStatic(strEndHeader, sizeof(strEndHeader)-1);
			static sl_char8 strEndCpp[] = "\t\tSLIB_DEFINE_MENU_END\r\n\r\n";
			sbCpp.addStatic(strEndCpp, sizeof(strEndCpp)-1);
		}
	}
	
	
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

sl_bool SAppDocument::_generateMenusCpp_Item(const String& parentName, int parentPlatforms, SAppMenuResourceItem* item, StringBuffer& sbHeader, StringBuffer& sbCpp, int tabLevel)
{
	String header, footer;
	if (item->platformFlags != parentPlatforms) {
		if (item->platformFlags == SAppMenuResourceItem::no_mac) {
			header = "#if !defined(SLIB_PLATFORM_IS_MACOS)\r\n";
			footer = "#endif\r\n";
		} else if (item->platformFlags == SAppMenuResourceItem::no_windows) {
			header = "#if !defined(SLIB_PLATFORM_IS_WIN32)\r\n";
			footer = "#endif\r\n";
		} else if (item->platformFlags == SAppMenuResourceItem::no_linux) {
			header = "#if !defined(SLIB_PLATFORM_IS_LINUX)\r\n";
			footer = "#endif\r\n";
		} else if (item->platformFlags != SAppMenuResourceItem::all_platforms) {
			String s;
			if (item->platformFlags & SAppMenuResourceItem::mac) {
				s = "defined(SLIB_PLATFORM_IS_MACOS)";
			}
			if (item->platformFlags & SAppMenuResourceItem::windows) {
				if (s.isNotEmpty()) {
					s += " | ";
				}
				s += "defined(SLIB_PLATFORM_IS_WIN32)";
			}
			if (item->platformFlags & SAppMenuResourceItem::linux) {
				if (s.isNotEmpty()) {
					s += " | ";
				}
				s += "defined(SLIB_PLATFORM_IS_LINUX)";
			}
			header = String::format("#if %s\r\n", s);
			footer = "#endif\r\n";
		}
	}
	
	if (item->name.isNotEmpty()) {
		sbHeader.add(header);
	}
	sbCpp.add(header);
	String tab('\t', tabLevel);
	
	if (item->type != SAppMenuResourceItem::typeSeparator) {
		if (!(_checkStringValueAvailable(item->title, item->element))) {
			return sl_false;
		}
		if (!(_checkDrawableValueImage(item->icon, item->element))) {
			return sl_false;
		}
		if (!(_checkDrawableValueImage(item->checkedIcon, item->element))) {
			return sl_false;
		}
	}
	
	if (item->type == SAppMenuResourceItem::typeSubmenu) {
		
		sbHeader.add(tab);
		sbHeader.add(String::format("SLIB_DECLARE_SUBMENU(%s)%n", item->name));
		
		sbCpp.add(tab);
		if (!(item->icon.flagDefined) && !(item->checkedIcon.flagDefined)) {
			sbCpp.add(String::format("SLIB_DEFINE_SUBMENU(%s, %s, %s)%n", parentName, item->name, item->title.getAccessString()));
		} else {
			sbCpp.add(String::format("SLIB_DEFINE_SUBMENU(%s, %s, %s, %s, %s)%n", parentName, item->name, item->title.getAccessString(), item->icon.getImageAccessString(), item->checkedIcon.getImageAccessString()));
		}
		
		ListLocker< Ref<SAppMenuResourceItem> > items(item->children);
		for (sl_size i = 0; i < items.count; i++) {
			Ref<SAppMenuResourceItem>& childItem = items[i];
			if (childItem.isNotNull()) {
				_generateMenusCpp_Item(item->name, item->platformFlags, childItem.ptr, sbHeader, sbCpp, tabLevel + 1);
			}
		}
		
	} else if (item->type == SAppMenuResourceItem::typeSeparator) {
		
		if (item->name.isNotEmpty()) {
			sbHeader.add(tab);
			sbHeader.add(String::format("SLIB_DECLARE_MENU_SEPARATOR(%s)%n", item->name));
			sbCpp.add(tab);
			sbCpp.add(String::format("SLIB_DEFINE_MENU_SEPARATOR(%s, %s)%n", parentName, item->name));
		} else {
			sbCpp.add(tab);
			sbCpp.add(String::format("SLIB_DEFINE_MENU_SEPARATOR_NONAME(%s)%n", parentName));
		}
		
	} else {
		
		sbHeader.add(tab);
		sbHeader.add(String::format("SLIB_DECLARE_MENU_ITEM(%s)%n", item->name));
		
		sbCpp.add(tab);
		String strShortcutKey = _getShortcutKeyDefinitionString(item->shortcutKey, sl_false);
		String strMacShortcutKey = _getShortcutKeyDefinitionString(item->macShortcutKey, sl_true);
		if (strShortcutKey != strMacShortcutKey) {
			strShortcutKey = String::format("SLIB_IF_PLATFORM_IS_MACOS(%s, %s)", strMacShortcutKey, strShortcutKey);
		}
		if (!(item->icon.flagDefined) && !(item->checkedIcon.flagDefined)) {
			sbCpp.add(String::format("SLIB_DEFINE_MENU_ITEM(%s, %s, %s, %s)%n", parentName, item->name, item->title.getAccessString(), strShortcutKey));
		} else {
			sbCpp.add(String::format("SLIB_DEFINE_MENU_ITEM(%s, %s, %s, %s, %s, %s)%n", parentName, item->name, item->title.getAccessString(), strShortcutKey, item->icon.getImageAccessString(), item->checkedIcon.getImageAccessString()));
		}
	}
	if (item->name.isNotEmpty()) {
		sbHeader.add(footer);
	}
	sbCpp.add(footer);
	
	return sl_true;
	
}

Ref<Menu> SAppDocument::_getMenuValue(const SAppMenuValue& value)
{
	if (!(value.flagDefined) || value.flagNull) {
		return Ref<Menu>::null();
	}
	Ref<SAppMenuResource> menuRes = m_menus.getValue(value.resourceName, Ref<SAppMenuResource>::null());
	if (menuRes.isNull()) {
		return Ref<Menu>::null();
	}
	Ref<Menu> menu = Menu::create();
	if (menu.isNull()) {
		_logError(_g_sdev_sapp_error_menu_not_found.arg(value.resourceName));
		return Ref<Menu>::null();
	}
	ListLocker< Ref<SAppMenuResourceItem> > items(menuRes->children);
	for (sl_size i = 0; i < items.count; i++) {
		Ref<SAppMenuResourceItem>& item = items[i];
		if (item.isNotNull()) {
			if (!(_getMenuValue_Item(menu, item.ptr))) {
				return Ref<Menu>::null();
			}
		}
	}
	return menu;
}

sl_bool SAppDocument::_getMenuValue_Item(const Ref<Menu>& parent, SAppMenuResourceItem* item)
{
	if (item->platformFlags & SAppMenuResourceItem::mac) {
#if !defined(SLIB_PLATFORM_IS_MACOS)
		return sl_true;
#endif
	}
	if (item->platformFlags & SAppMenuResourceItem::windows) {
#if !defined(SLIB_PLATFORM_IS_WIN32)
		return sl_true;
#endif
	}
	if (item->platformFlags & SAppMenuResourceItem::linux) {
#if !defined(SLIB_PLATFORM_IS_LINUX)
		return sl_true;
#endif
	}
	
	if (item->type != SAppMenuResourceItem::typeSeparator) {
		if (!(_checkStringValueAvailable(item->title, item->element))) {
			return sl_false;
		}
		if (!(_checkDrawableValueImage(item->icon, item->element))) {
			return sl_false;
		}
		if (!(_checkDrawableValueImage(item->checkedIcon, item->element))) {
			return sl_false;
		}
	}
	
	if (item->type == SAppMenuResourceItem::typeSubmenu) {

		Ref<Menu> submenu = Menu::create();
		if (submenu.isNull()) {
			return sl_false;
		}
		if (!(item->icon.flagDefined) && !(item->checkedIcon.flagDefined)) {
			parent->addSubmenu(submenu, _getStringValue(item->title));
		} else {
			parent->addSubmenu(submenu, _getStringValue(item->title), _getImageValue(item->icon), _getImageValue(item->checkedIcon));
		}
		
		ListLocker< Ref<SAppMenuResourceItem> > items(item->children);
		for (sl_size i = 0; i < items.count; i++) {
			Ref<SAppMenuResourceItem>& childItem = items[i];
			if (childItem.isNotNull()) {
				if (!(_getMenuValue_Item(submenu.ptr, childItem.ptr))) {
					return sl_false;
				}
			}
		}
		
	} else if (item->type == SAppMenuResourceItem::typeSeparator) {
		
		parent->addSeparator();
		
	} else {
		
		KeycodeAndModifiers km = SLIB_IF_PLATFORM_IS_MACOS(item->macShortcutKey, item->shortcutKey);
		if (!(item->icon.flagDefined) && !(item->checkedIcon.flagDefined)) {
			parent->addMenuItem(_getStringValue(item->title), km);
		} else {
			parent->addMenuItem(_getStringValue(item->title), km, _getImageValue(item->icon), _getImageValue(item->checkedIcon));
		}
	}
	
	return sl_true;
	
}

sl_bool SAppDocument::_checkMenuValueAvailable(SAppMenuValue& value, const Ref<XmlElement>& elementError)
{
	if (!(value.flagDefined)) {
		return sl_true;
	}
	if (value.flagNull) {
		return sl_true;
	}
	if (m_menus.contains(value.resourceName)) {
		return sl_true;
	} else {
		_logError(elementError, _g_sdev_sapp_error_menu_not_found.arg(value.resourceName));
		return sl_false;
	}
}

/***************************************************
				Layout Resources
***************************************************/

#define LOG_ERROR_LAYOUT_ATTR(NAME) \
	_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str_##NAME));

#define PARSE_AND_CHECK_LAYOUT_ATTR(ATTR, NAME) \
	String str_##NAME = item->getXmlAttribute(#NAME); \
	if (!(ATTR NAME.parse(str_##NAME))) { \
		LOG_ERROR_LAYOUT_ATTR(NAME) \
		return sl_false; \
	}

#define LOG_ERROR_LAYOUT_XML_ATTR(XML, NAME) \
	_logError(XML, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str_##NAME));

#define PARSE_AND_CHECK_LAYOUT_XML_ATTR(XML, ATTR, NAME) \
	String str_##NAME = XML->getAttribute(#NAME); \
	if (!(ATTR NAME.parse(str_##NAME))) { \
		LOG_ERROR_LAYOUT_XML_ATTR(XML, NAME) \
		return sl_false; \
	}

#define GENERATE_CPP_SET_LAYOUT_ATTR(ATTR, NAME, FUNC, SB) \
	if (ATTR NAME.flagDefined) { \
		SB.add(String::format("%s%s->" #FUNC "(%s);%n", strTab, name, ATTR NAME.getAccessString())); \
	}

#define GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(ATTR, NAME, FUNC, SB) \
	if (ATTR NAME.flagDefined) { \
		SB.add(String::format("%s%s->" #FUNC "(%s, sl_false);%n", strTab, name, ATTR NAME.getAccessString())); \
	}

#define GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(ATTR, NAME, FUNC, SB_INIT, SB_LAYOUT) \
	if (ATTR NAME.flagDefined) { \
		if (ATTR NAME.isNeededOnLayoutFunction()) { \
			SB_LAYOUT.add(String::format("%s%s->" #FUNC "(%s, sl_false);%n", strTab, name, ATTR NAME.getAccessString())); \
		} else { \
			SB_INIT.add(String::format("%s%s->" #FUNC "(%s, sl_false);%n", strTab, name, ATTR NAME.getAccessString())); \
		} \
	}

#define SIMULATE_LAYOUT_SET_LAYOUT_ATTR(ATTR, NAME, FUNC, TO_VALUE) \
	if (!flagOnLayout && ATTR NAME.flagDefined) { \
		view->FUNC(TO_VALUE##Value(ATTR NAME)); \
	}

#define SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(ATTR, NAME, FUNC, TO_VALUE) \
	if (!flagOnLayout && ATTR NAME.flagDefined) { \
		view->FUNC(TO_VALUE##Value(ATTR NAME), sl_false); \
	}

#define SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(ATTR, NAME, FUNC) \
	if (ATTR NAME.flagDefined) { \
		if (flagOnLayout) { \
			view->FUNC(_getDimensionValue(ATTR NAME), sl_false); \
		} \
	}

#define SIMULATE_LAYOUT_SET_LAYOUT_ATTR_DRAWABLE(ATTR, NAME, FUNC) \
	if (flagOnLayout && ATTR NAME.flagDefined) { \
		view->FUNC(_getDrawableValue(ATTR NAME), sl_false); \
	}

#define _getValue(x) x.value

#define RADIOGROUP_NAME_PREFIX "radioGroup_"

sl_bool SAppDocument::_parseLayoutStyle(const Ref<XmlElement>& element)
{
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutStyle> style = new SAppLayoutStyle;
	if (style.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	style->element = element;

	String name = element->getAttribute("name").trim();
	if (name.isEmpty()) {
		_logError(element, _g_sdev_sapp_error_resource_layout_name_is_empty);
		return sl_false;
	}
	if (m_layoutStyles.contains(name)) {
		_logError(element, _g_sdev_sapp_error_resource_layout_name_redefined.arg(name));
		return sl_false;
	}
	style->name = name;
	
	if (!(m_layoutStyles.put(name, style))) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}

	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResource(const Ref<XmlElement>& element)
{
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutResource> layout = new SAppLayoutResource;
	if (layout.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	layout->element = element;

	int type;
	String strType = element->getAttribute("type");
	if (strType.isEmpty() || strType == "view") {
		type = SAppLayoutResource::typeView;
	} else if (strType == "window") {
		type = SAppLayoutResource::typeWindow;
	} else if (strType == "mobile-page") {
		type = SAppLayoutResource::typeMobilePage;
	} else {
		_logError(element, _g_sdev_sapp_error_resource_layout_type_invalid.arg(strType));
		return sl_false;
	}
	layout->type = type;
	
	String name = element->getAttribute("name");
	if (name.isEmpty()) {
		_logError(element, _g_sdev_sapp_error_resource_layout_name_is_empty);
		return sl_false;
	}
	if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
		_logError(element, _g_sdev_sapp_error_resource_layout_name_invalid.arg(name));
		return sl_false;
	}
	if (m_layouts.contains(name)) {
		_logError(element, _g_sdev_sapp_error_resource_layout_name_redefined.arg(name));
		return sl_false;
	}
	layout->name = name;
	
	PARSE_AND_CHECK_LAYOUT_XML_ATTR(element, layout->, customUnit)
	if (!(layout->customUnit.checkCustomUnit())) {
		LOG_ERROR_LAYOUT_ATTR(customUnit)
		return sl_false;
	}
	
	if (!(_parseLayoutResourceItem(layout.ptr, layout.ptr, sl_null))) {
		return sl_false;
	}
	
	if (!(m_layouts.put(name, layout))) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceItem(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	String strStyles = element->getAttribute("styles").trim();
	if (strStyles.isNotEmpty()) {
		ListLocker<String> arr(strStyles.split(","));
		for (sl_size i = 0; i < arr.count; i++) {
			String s = arr[i].trim();
			Ref<SAppLayoutStyle> style;
			m_layoutStyles.get(s, &style);
			if (style.isNotNull()) {
				item->styles.add(style);
			} else {
				_logError(element, _g_sdev_sapp_error_layout_style_not_found.arg(s));
				return sl_false;
			}
		}
	}
	
	if (parent) {
		String name = element->getAttribute("name");
		if (name.isNotEmpty()) {
			if (!(SDevUtil::checkName(name.getData(), name.getLength()))) {
				_logError(element, _g_sdev_sapp_error_resource_layout_name_invalid.arg(name));
				return sl_false;
			}
			if (layout->itemsByName.contains(name)) {
				_logError(element, _g_sdev_sapp_error_resource_layout_name_redefined.arg(name));
				return sl_false;
			}
		} else {
			name = layout->getAutoIncreasingName(item->type);
			item->flagGeneratedName = sl_true;
		}
		item->name = name;
	}
	
	if (item->type == SAppLayoutResource::typeWindow) {
		if (!(_parseLayoutResourceWindowAttributes(layout))) {
			return sl_false;
		}
		if (!(_parseLayoutResourceItemChildren(layout, item))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeMobilePage) {
		if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
			return sl_false;
		}
		if (!(layout->viewAttrs->width.flagDefined)) {
			layout->viewAttrs->width.flagDefined = sl_true;
			layout->viewAttrs->width.amount = 1;
			layout->viewAttrs->width.unit = SAppDimensionValue::SW;
		}
		if (!(layout->viewAttrs->height.flagDefined)) {
			layout->viewAttrs->height.flagDefined = sl_true;
			layout->viewAttrs->height.amount = 1;
			layout->viewAttrs->height.unit = SAppDimensionValue::SH;
		}
		if (!(_parseLayoutResourceItemChildren(layout, item))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeView) {
		item->className = "slib::View";
		if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
			return sl_false;
		}
		if (!(_parseLayoutResourceItemChildren(layout, item))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeViewGroup) {
		item->className = "slib::ViewGroup";
		if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
			return sl_false;
		}
		if (!(_parseLayoutResourceItemChildren(layout, item))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeButton) {
		item->className = "slib::Button";
		if (!(_parseLayoutResourceButtonAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeLabel) {
		item->className = "slib::LabelView";
		if (!(_parseLayoutResourceLabelAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeCheck) {
		item->className = "slib::CheckBox";
		if (!(_parseLayoutResourceCheckAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeRadio) {
		item->className = "slib::RadioButton";
		if (!(_parseLayoutResourceRadioAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeEdit) {
		item->className = "slib::EditView";
		if (!(_parseLayoutResourceEditAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typePassword) {
		item->className = "slib::PasswordView";
		if (!(_parseLayoutResourceEditAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeTextArea) {
		item->className = "slib::TextArea";
		if (!(_parseLayoutResourceEditAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeImage) {
		item->className = "slib::ImageView";
		if (!(_parseLayoutResourceImageAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeSelect) {
		item->className = "slib::SelectView";
		if (!(_parseLayoutResourceSelectAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeScroll) {
		item->className = "slib::ScrollView";
		if (!(_parseLayoutResourceScrollAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeLinear) {
		item->className = "slib::LinearView";
		if (!(_parseLayoutResourceLinearAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeList) {
		item->className = "slib::ListView";
		if (!(_parseLayoutResourceListAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeListReport) {
		item->className = "slib::ListReportView";
		if (!(_parseLayoutResourceListReportAttributes(layout, item, parent))) {
			return sl_false;
		}
	} else if (item->type == SAppLayoutResource::typeRender) {
		item->className = "slib::RenderView";
	} else if (item->type == SAppLayoutResource::typeTab) {
		item->className = "slib::TabView";
	} else if (item->type == SAppLayoutResource::typeTree) {
		item->className = "slib::TreeView";
	} else if (item->type == SAppLayoutResource::typeWeb) {
		item->className = "slib::WebView";
	} else {
		return sl_false;
	}
	
	String customClassName = item->getXmlAttribute("class").trim();
	if (customClassName.isNotNull()) {
		item->className = customClassName;
	}
	
	if (customClassName.isNotEmpty()) {
		if (!(layout->customClasses.put(customClassName, sl_true))) {
			_logError(element, _g_sdev_sapp_error_out_of_memory);
			return sl_false;
		}
	}
	
	if (parent) {
		if (!(layout->itemsByName.put(item->name, item))) {
			_logError(element, _g_sdev_sapp_error_out_of_memory);
			return sl_false;
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceItemChildren(SAppLayoutResource* layout, SAppLayoutResourceItem* item)
{
	ListLocker< Ref<XmlElement> > children(item->element->getChildElements());
	for (sl_size i = 0; i < children.count; i++) {
		const Ref<XmlElement>& child = children[i];
		if (child.isNotNull()) {
			Ref<SAppLayoutResourceItem> childItem = _parseLayoutResourceItemChild(layout, item, child);
			if (childItem.isNull()) {
				return sl_false;
			}
			if (!(item->children.add(childItem))) {
				_logError(item->element, _g_sdev_sapp_error_out_of_memory);
				return sl_false;
			}
		}
	}
	return sl_true;
}

Ref<SAppLayoutResourceItem> SAppDocument::_parseLayoutResourceItemChild(SAppLayoutResource* layout, SAppLayoutResourceItem* parentItem, const Ref<XmlElement>& element)
{
	String strType = element->getName();
	int type = SAppLayoutResource::getTypeFromName(strType);
	if (type == SAppLayoutResource::typeUnknown) {
		_logError(element, _g_sdev_sapp_error_resource_layout_type_invalid.arg(strType));
		return Ref<SAppLayoutResourceItem>::null();
	}
	
	Ref<SAppLayoutResourceItem> childItem = new SAppLayoutResourceItem;
	if (childItem.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return Ref<SAppLayoutResourceItem>::null();
	}
	
	childItem->type = type;
	childItem->element = element;
	
	if (!(_parseLayoutResourceItem(layout, childItem.ptr, parentItem))) {
		return Ref<SAppLayoutResourceItem>::null();
	}
	
	return childItem;
}

sl_bool SAppDocument::_generateLayoutsCpp(const String& targetPath)
{
	_log(_g_sdev_sapp_log_generate_cpp_layouts_begin);
	
	StringBuffer sbHeader, sbCpp;
	sbHeader.add(String::format(
								"#ifndef CHECKHEADER_RESOURCE_UI_%s%n"
								"#define CHECKHEADER_RESOURCE_UI_%s%n%n"
								"#include <slib/ui/resource.h>%n"
								, m_conf.generate_cpp_namespace));

	{
		ListLocker<String> includes(m_conf.generate_cpp_layout_include_headers);
		for (sl_size i = 0; i < includes.count; i++) {
			if (includes[i].isNotEmpty()) {
				sbHeader.add(String::format("#include \"%s\"%n", includes[i]));
			}
		}
	}

	sbHeader.add(String::format(
								"%n"
								"#include <slib/ui/resource.h>%n%n"
								"namespace %s%n"
								"{%n\tnamespace ui%n\t{%n%n"
								, m_conf.generate_cpp_namespace));
	
	sbCpp.add(String::format(
							 "#include \"layouts.h\"%n"
							 "#include \"strings.h\"%n"
							 "#include \"drawables.h\"%n"
							 "#include \"menus.h\"%n%n"
							 "namespace %s%n"
							 "{%n\tnamespace ui%n\t{%n%n"
							 , m_conf.generate_cpp_namespace));
	
	
	Iterator< Pair<String, Ref<SAppLayoutResource> > > iterator = m_layouts.iterator();
	Pair< String, Ref<SAppLayoutResource> > pair;
	
	while (iterator.next(&pair)) {
		if (pair.value.isNotNull()) {
			
			Ref<SAppLayoutResource> layout = pair.value;
			
			if (layout->type == SAppLayoutResource::typeWindow) {
				sbHeader.add(String::format("\t\tSLIB_DECLARE_WINDOW_LAYOUT_BEGIN(%s)%n", pair.key));
				sbCpp.add(String::format("\t\tSLIB_DEFINE_WINDOW_LAYOUT(%s)%n%n", pair.key));
			} else if (layout->type == SAppLayoutResource::typeMobilePage) {
				sbHeader.add(String::format("\t\tSLIB_DECLARE_MOBILE_PAGE_LAYOUT_BEGIN(%s)%n", pair.key));
				sbCpp.add(String::format("\t\tSLIB_DEFINE_MOBILE_PAGE_LAYOUT(%s)%n%n", pair.key));
			} else if (layout->type == SAppLayoutResource::typeView) {
				sbHeader.add(String::format("\t\tSLIB_DECLARE_VIEW_LAYOUT_BEGIN(%s)%n", pair.key));
				sbCpp.add(String::format("\t\tSLIB_DEFINE_VIEW_LAYOUT(%s)%n%n", pair.key));
			} else {
				return sl_false;
			}
			
			sbCpp.add(String::format("\t\tvoid %s::initialize()%n\t\t{%n", pair.key));
			
			{
				ListItems<String> radioGroups(layout->radioGroups.keys());
				for (sl_size i = 0; i < radioGroups.count; i++) {
					sbHeader.add(String::format("\t\t\tslib::Ref<slib::RadioGroup> " RADIOGROUP_NAME_PREFIX "%s;%n", radioGroups[i]));
					sbCpp.add(String::format("\t\t\t" RADIOGROUP_NAME_PREFIX "%s = new slib::RadioGroup;%n", radioGroups[i]));
				}
				if (radioGroups.count > 0) {
					sbHeader.add("\r\n");
					sbCpp.add("\r\n");
				}
			}
			
			StringBuffer sbLayout;
			
			if (layout->customUnit.flagDefined) {
				if (layout->customUnit.isNeededOnLayoutFunction()) {
					sbLayout.add(String::format("%n\t\t\tsetCustomUnitLength(%s);%n", layout->customUnit.getAccessString()));
				} else {
					sbCpp.add(String::format("%n\t\t\tsetCustomUnitLength(%s);%n%n", layout->customUnit.getAccessString()));
				}
			}
			
			if (!(_generateLayoutsCppItem(sl_null, layout.ptr, sbHeader, sbCpp, sbLayout, String::null()))) {
				return sl_false;
			}
			
			sbCpp.add(String::format("\t\t}%n%n\t\tvoid %s::layoutViews(sl_real CONTENT_WIDTH, sl_real CONTENT_HEIGHT)%n\t\t{%n", pair.key));
			sbCpp.link(sbLayout);
			static sl_char8 strEndCpp[] = "\t\t}\r\n\r\n";
			sbCpp.addStatic(strEndCpp, sizeof(strEndCpp)-1);
			
			if (layout->type == SAppLayoutResource::typeWindow) {
				static sl_char8 strEndHeader[] = "\t\tSLIB_DECLARE_WINDOW_LAYOUT_END\r\n\r\n";
				sbHeader.addStatic(strEndHeader, sizeof(strEndHeader)-1);
			} else if (layout->type == SAppLayoutResource::typeView) {
				static sl_char8 strEndHeader[] = "\t\tSLIB_DECLARE_VIEW_LAYOUT_END\r\n\r\n";
				sbHeader.addStatic(strEndHeader, sizeof(strEndHeader)-1);
			}
		}
	}
		
	sbHeader.add("\t}\r\n}\r\n\r\n#endif");
	sbCpp.add("\t}\r\n}\r\n");
	
	String pathHeader = targetPath + "/layouts.h";
	String contentHeader = sbHeader.merge();
	if (!(File::writeAllTextUTF8(pathHeader, contentHeader))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathHeader));
		return sl_false;
	}
	
	String pathCpp = targetPath + "/layouts.cpp";
	String contentCpp = sbCpp.merge();
	if (!(File::writeAllTextUTF8(pathCpp, contentCpp))) {
		_logError(_g_sdev_sapp_error_file_write_failed.arg(pathCpp));
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppItem(SAppLayoutResourceItem* parent, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	String name;
	if (parent) {
		name = item->name;
		sbDeclare.add(String::format("\t\t\tslib::Ref<%s> %s;%n", item->className, name));
		sbDefineInit.add(String::format("\t\t\t%2$s = new %1$s;%n", item->className, name));
		switch (item->type) {
			case SAppLayoutResource::typeView:
			case SAppLayoutResource::typeViewGroup:
				if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
					return sl_false;
				}
				if (addStatement.isNotNull()) {
					sbDefineInit.add(addStatement);
				}
				break;
			case SAppLayoutResource::typeButton:
				if (!(_generateLayoutsCppButton(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeLabel:
				if (!(_generateLayoutsCppLabelView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeCheck:
				if (!(_generateLayoutsCppCheckBox(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeRadio:
				if (!(_generateLayoutsCppRadioButton(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeEdit:
				if (!(_generateLayoutsCppEditView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typePassword:
				if (!(_generateLayoutsCppEditView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeTextArea:
				if (!(_generateLayoutsCppEditView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeImage:
				if (!(_generateLayoutsCppImageView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeSelect:
				if (!(_generateLayoutsCppSelectView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeScroll:
				if (!(_generateLayoutsCppScrollView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeLinear:
				if (!(_generateLayoutsCppLinearView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeList:
				if (!(_generateLayoutsCppListView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeListReport:
				if (!(_generateLayoutsCppListReportView(name, item, sbDeclare, sbDefineInit, sbDefineLayout, addStatement))) {
					return sl_false;
				}
				break;
			case SAppLayoutResource::typeRender:
				break;
			case SAppLayoutResource::typeTab:
				break;
			case SAppLayoutResource::typeTree:
				break;
			case SAppLayoutResource::typeWeb:
				break;
			default:
				return sl_false;
		}

	} else {
		if (item->type == SAppLayoutResource::typeWindow) {
			if (!(_generateLayoutsCppWindowAttributes("this", item, sbDefineInit))) {
				return sl_false;
			}
		} else if (item->type == SAppLayoutResource::typeMobilePage) {
			if (!(_generateLayoutsCppViewAttributes("this", item, sbDefineInit, sbDefineInit))) {
				return sl_false;
			}
		} else if (item->type == SAppLayoutResource::typeView) {
			if (!(_generateLayoutsCppViewAttributes("this", item, sbDefineInit, sbDefineInit))) {
				return sl_false;
			}
		} else {
			return sl_false;
		}
		static sl_char8 strEnd[] = "\r\n";
		sbDefineInit.addStatic(strEnd, sizeof(strEnd)-1);
		name = "m_contentView";
	}
	
	if (item->type == SAppLayoutResource::typeWindow || item->type == SAppLayoutResource::typeMobilePage || item->type == SAppLayoutResource::typeView || item->type == SAppLayoutResource::typeViewGroup || item->type == SAppLayoutResource::typeLinear) {
		ListLocker< Ref<SAppLayoutResourceItem> > children(item->children);
		for (sl_size i = 0; i < children.count; i++) {
			Ref<SAppLayoutResourceItem>& child = children[i];
			if (child.isNotNull()) {
				String addChildStatement = String::format("\t\t\t%s->addChild(%s, sl_false);%n%n", name, child->name);
				if (!(_generateLayoutsCppItem(item, child.ptr, sbDeclare, sbDefineInit, sbDefineLayout, addChildStatement))) {
					return sl_false;
				}
			}
		}
	}

	return sl_true;
}

void SAppDocument::_simulateLayoutInWindow(SAppLayoutResource* layout)
{
	Ref<SAppLayoutSimulationWindow> window = new SAppLayoutSimulationWindow;
	if (window.isNotNull()) {
		window->open(this, layout);
	}
}

void SAppDocument::_registerLayoutSimulationWindow(const Ref<SAppLayoutSimulationWindow>& window)
{
	m_layoutSimulationWindows.add(window);
}

void SAppDocument::_removeLayoutSimulationWindow(const Ref<SAppLayoutSimulationWindow>& window)
{
	m_layoutSimulationWindows.removeValue(window);
}

Ref<View> SAppDocument::_simulateLayoutCreateOrLayoutView(SAppLayoutSimulationWindow* simulator, SAppLayoutResourceItem* item, View* parent, sl_bool flagOnLayout)
{
	Ref<View> view;
	if (parent) {
		if (flagOnLayout) {
			view = simulator->getViewByName(item->name);
		}
		switch (item->type) {
			case SAppLayoutResource::typeView:
			case SAppLayoutResource::typeViewGroup:
				if (!flagOnLayout) {
					if (item->type == SAppLayoutResource::typeViewGroup) {
						view = new ViewGroup;
					} else {
						view = new View;
					}
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetViewAttributes(simulator, view.ptr, item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeButton:
				if (!flagOnLayout) {
					view = new Button;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetButtonAttributes(simulator, (Button*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeLabel:
				if (!flagOnLayout) {
					view = new LabelView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetLabelAttributes(simulator, (LabelView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeCheck:
				if (!flagOnLayout) {
					view = new CheckBox;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetCheckAttributes(simulator, (CheckBox*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeRadio:
				if (!flagOnLayout) {
					view = new RadioButton;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetRadioAttributes(simulator, (RadioButton*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeEdit:
				if (!flagOnLayout) {
					view = new EditView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetEditAttributes(simulator, (EditView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typePassword:
				if (!flagOnLayout) {
					view = new PasswordView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetEditAttributes(simulator, (EditView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeTextArea:
				if (!flagOnLayout) {
					view = new TextArea;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetEditAttributes(simulator, (EditView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeImage:
				if (!flagOnLayout) {
					view = new ImageView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetImageAttributes(simulator, (ImageView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeSelect:
				if (!flagOnLayout) {
					view = new SelectView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetSelectAttributes(simulator, (SelectView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeScroll:
				if (!flagOnLayout) {
					view = new ScrollView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetScrollAttributes(simulator, (ScrollView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeLinear:
				if (!flagOnLayout) {
					view = new LinearView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetLinearAttributes(simulator, (LinearView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeList:
				if (!flagOnLayout) {
					view = new ListView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetListAttributes(simulator, (ListView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeListReport:
				if (!flagOnLayout) {
					view = new ListReportView;
				}
				if (view.isNotNull()) {
					if (!(_simulateLayoutSetListReportAttributes(simulator, (ListReportView*)(view.ptr), item, flagOnLayout))) {
						return Ref<View>::null();
					}
				}
				break;
			case SAppLayoutResource::typeRender:
				break;
			case SAppLayoutResource::typeTab:
				break;
			case SAppLayoutResource::typeTree:
				break;
			case SAppLayoutResource::typeWeb:
				break;
			default:
				return Ref<View>::null();
		}
		if (view.isNull()) {
			return Ref<View>::null();
		}
		if (!flagOnLayout) {
			simulator->registerViewByName(item->name, view);
		}
	} else {
		Ref<SAppLayoutResource> layout = simulator->getLayout();
		if (flagOnLayout) {
			if (item->type != SAppLayoutResource::typeWindow) {
				view = simulator->getViewByName("m_contentView");
			}
		}
		if (item->type == SAppLayoutResource::typeWindow) {
			view = simulator->getContent();
			if (!flagOnLayout) {
				if (view.isNotNull()) {
					Size size = UI::getScreenSize();
					m_layoutSimulationParams.screenWidth = size.x;
					m_layoutSimulationParams.screenHeight = size.y;
					m_layoutSimulationParams.viewportWidth = view->getWidth();
					m_layoutSimulationParams.viewportHeight = view->getHeight();
					m_layoutSimulationParams.customUnit = 1;
					if (layout.isNotNull() && layout->customUnit.flagDefined && !(layout->customUnit.isNeededOnLayoutFunction())) {
						m_layoutSimulationParams.customUnit = _getDimensionValue(layout->customUnit);
					}
					if (!(_simulateLayoutSetWindowAttributes(simulator, item))) {
						return Ref<View>::null();
					}
					if (!(_simulateLayoutSetRootViewAttributes(view.ptr, item))) {
						return Ref<View>::null();
					}
				}
			} else {
				if (layout.isNotNull() && layout->customUnit.flagDefined && layout->customUnit.isNeededOnLayoutFunction()) {
					m_layoutSimulationParams.customUnit = _getDimensionValue(layout->customUnit);
				}
			}
		} else if (item->type == SAppLayoutResource::typeMobilePage || item->type == SAppLayoutResource::typeView) {
			if (!flagOnLayout) {
				view = new ViewGroup;
			}
			if (view.isNotNull()) {
				Size size = simulator->getClientSize();
				m_layoutSimulationParams.screenWidth = size.x;
				m_layoutSimulationParams.screenHeight = size.y;
				m_layoutSimulationParams.viewportWidth = view->getWidth();
				m_layoutSimulationParams.viewportHeight = view->getHeight();
				m_layoutSimulationParams.customUnit = 1;
				if (layout.isNotNull() && layout->customUnit.flagDefined) {
					m_layoutSimulationParams.customUnit = _getDimensionValue(layout->customUnit);
				}
				if (!(_simulateLayoutSetViewAttributes(simulator, view.ptr, item, flagOnLayout))) {
					return Ref<View>::null();
				}
				m_layoutSimulationParams.viewportWidth = view->getWidth();
				m_layoutSimulationParams.viewportHeight = view->getHeight();
			}
		} else {
			return Ref<View>::null();
		}
		if (!flagOnLayout) {
			if (view.isNotNull()) {
				if (item->type != SAppLayoutResource::typeWindow) {
					simulator->registerViewByName("m_contentView", view);
				}
			}
		}

	}
	
	if (view.isNotNull()) {
		if (item->type == SAppLayoutResource::typeWindow || item->type == SAppLayoutResource::typeMobilePage || item->type == SAppLayoutResource::typeView || item->type == SAppLayoutResource::typeViewGroup || item->type == SAppLayoutResource::typeLinear) {
			ListLocker< Ref<SAppLayoutResourceItem> > children(item->children);
			for (sl_size i = 0; i < children.count; i++) {
				Ref<SAppLayoutResourceItem>& child = children[i];
				if (child.isNotNull()) {
					Ref<View> childView = _simulateLayoutCreateOrLayoutView(simulator, child.ptr, view.ptr, flagOnLayout);
					if (childView.isNotNull()) {
						view->addChild(childView, sl_false);
					} else {
						return Ref<View>::null();
					}
				}
			}
		}
	}
	
	return view;
	
}

sl_real SAppDocument::_getDimensionValue(SAppDimensionValue& value)
{
	if (!(value.flagDefined)) {
		return 0;
	}
	switch (value.unit) {
		case SAppDimensionValue::CUSTOM:
			return value.amount * m_layoutSimulationParams.customUnit;
		case SAppDimensionValue::PX:
			return value.amount;
		case SAppDimensionValue::SW:
			return value.amount * m_layoutSimulationParams.screenWidth;
		case SAppDimensionValue::SH:
			return value.amount * m_layoutSimulationParams.screenHeight;
		case SAppDimensionValue::SMIN:
			return value.amount * SLIB_MIN(m_layoutSimulationParams.screenWidth, m_layoutSimulationParams.screenHeight);
		case SAppDimensionValue::SMAX:
			return value.amount * SLIB_MAX(m_layoutSimulationParams.screenWidth, m_layoutSimulationParams.screenHeight);
		case SAppDimensionValue::VW:
			return value.amount * m_layoutSimulationParams.viewportWidth;
		case SAppDimensionValue::VH:
			return value.amount * m_layoutSimulationParams.viewportHeight;
		case SAppDimensionValue::VMIN:
			return value.amount * SLIB_MIN(m_layoutSimulationParams.viewportWidth, m_layoutSimulationParams.viewportHeight);
		case SAppDimensionValue::VMAX:
			return value.amount * SLIB_MAX(m_layoutSimulationParams.viewportWidth, m_layoutSimulationParams.viewportHeight);
	}
	return 0;
}

sl_bool SAppDocument::_parseLayoutResourceRootViewAttributes(SAppLayoutResource* item)
{
	
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutViewAttributes> attr = new SAppLayoutViewAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingLeft)
	if (!(attr->paddingLeft.checkForRootViewPosition())) {
		LOG_ERROR_LAYOUT_ATTR(paddingLeft)
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingTop)
	if (!(attr->paddingTop.checkForRootViewPosition())) {
		LOG_ERROR_LAYOUT_ATTR(paddingTop)
		return sl_false;
	}

	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingRight)
	if (!(attr->paddingRight.checkForRootViewPosition())) {
		LOG_ERROR_LAYOUT_ATTR(paddingRight)
		return sl_false;
	}

	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingBottom)
	if (!(attr->paddingBottom.checkForRootViewPosition())) {
		LOG_ERROR_LAYOUT_ATTR(paddingBottom)
		return sl_false;
	}
	
	SAppDimensionValue padding;
	PARSE_AND_CHECK_LAYOUT_ATTR(, padding)
	if (!(padding.checkForRootViewPosition())) {
		LOG_ERROR_LAYOUT_ATTR(padding)
		return sl_false;
	}
	if (padding.flagDefined) {
		if (!(attr->paddingLeft.flagDefined)) {
			attr->paddingLeft = padding;
		}
		if (!(attr->paddingTop.flagDefined)) {
			attr->paddingTop = padding;
		}
		if (!(attr->paddingRight.flagDefined)) {
			attr->paddingRight = padding;
		}
		if (!(attr->paddingBottom.flagDefined)) {
			attr->paddingBottom = padding;
		}
	}

	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, occurringClick)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, background)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, backgroundScale)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, backgroundAlign)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, backgroundColor)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontFamily)
	if (attr->fontFamily.flagDefined) {
		attr->finalFontFamily = attr->fontFamily;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontSize)
	if (!(attr->fontSize.checkForRootViewPosition())) {
		LOG_ERROR_LAYOUT_ATTR(fontSize)
		return sl_false;
	}
	if (attr->fontSize.flagDefined) {
		attr->finalFontSize = attr->fontSize;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontBold)
	if (attr->fontBold.flagDefined) {
		attr->finalFontBold = attr->fontBold.value;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontItalic)
	if (attr->fontItalic.flagDefined) {
		attr->finalFontItalic = attr->fontItalic.value;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontUnderline)
	if (attr->fontUnderline.flagDefined) {
		attr->finalFontUnderline = attr->fontUnderline.value;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollBars)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByMouse)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByTouch)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByMouseWheel)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByKeyboard)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, multiTouch)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, childInstances)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, doubleBuffering)
	
	item->viewAttrs = attr;
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppRootViewAttributes(SAppLayoutResourceItem* item, StringBuffer& sbDefineInit)
{
	SAppLayoutViewAttributes* attr = item->viewAttrs.ptr;
	String strTab = "\t\t\t";
	String name = "m_contentView";

	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingLeft, setPaddingLeft, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingTop, setPaddingTop, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingRight, setPaddingRight, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingBottom, setPaddingBottom, sbDefineInit)

	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, occurringClick, setOccurringClick, sbDefineInit)
	if (!(_checkDrawableValueAvailable(attr->background, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, background, setBackground, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundScale, setBackgroundScaleMode, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundAlign, setBackgroundAlignment, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundColor, setBackgroundColor, sbDefineInit)

	if (!(_checkStringValueAvailable(attr->fontFamily, item->element))) {
		return sl_false;
	}
	if (attr->fontFamily.flagDefined || attr->fontSize.flagDefined || attr->fontBold.flagDefined || attr->fontItalic.flagDefined || attr->fontUnderline.flagDefined) {
		String fontSize;
		if (attr->finalFontSize.flagDefined) {
			fontSize = attr->finalFontSize.getAccessString();
		} else {
			fontSize = "UI::getDefaultFontSize()";
		}
		if (attr->finalFontFamily.flagDefined) {
			sbDefineInit.add(String::format("%s%s->setFont(%s, ", strTab, name, attr->finalFontFamily.getAccessString()));
		} else {
			sbDefineInit.add(String::format("%s%s->setFontAttributes(", strTab, name));
		}
		sbDefineInit.add(String::format("%s, %s, %s, %s, sl_false);%n", fontSize, attr->finalFontBold?"sl_true":"sl_false", attr->finalFontItalic?"sl_true":"sl_false", attr->finalFontUnderline?"sl_true":"sl_false"));
	}
	
	if (attr->scrollBars.horizontalScrollBar) {
		if (attr->scrollBars.verticalScrollBar) {
			sbDefineInit.add(String::format("%s%s->createScrollBars(sl_false);%n", strTab, name));
		} else {
			sbDefineInit.add(String::format("%s%s->createHorizontalScrollBar(sl_false);%n", strTab, name));
		}
	} else {
		if (attr->scrollBars.verticalScrollBar) {
			sbDefineInit.add(String::format("%s%s->createVerticalScrollBar(sl_false);%n", strTab, name));
		}
	}
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByMouse, setContentScrollingByMouse, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByTouch, setContentScrollingByTouch, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByMouseWheel, setContentScrollingByMouseWheel, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByKeyboard, setContentScrollingByKeyboard, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, multiTouch, setMultiTouchMode, sbDefineInit)
	
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, childInstances, setCreatingChildInstances, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, doubleBuffering, setDoubleBuffering, sbDefineInit)
	
	return sl_true;
	
}

sl_bool SAppDocument::_simulateLayoutSetRootViewAttributes(View* view, SAppLayoutResourceItem* item)
{
	SAppLayoutViewAttributes* attr = item->viewAttrs.ptr;
	sl_bool flagOnLayout = sl_false;
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingLeft, setPaddingLeft, _getDimension)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingTop, setPaddingTop, _getDimension)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingRight, setPaddingRight, _getDimension)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, paddingBottom, setPaddingBottom, _getDimension)
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, occurringClick, setOccurringClick, _get)
	if (!(_checkDrawableValueAvailable(attr->background, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, background, setBackground, _getDrawable)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundScale, setBackgroundScaleMode, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundAlign, setBackgroundAlignment, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundColor, setBackgroundColor, _get)
	
	if (!(_checkStringValueAvailable(attr->fontFamily, item->element))) {
		return sl_false;
	}
	if (attr->fontFamily.flagDefined || attr->fontSize.flagDefined || attr->fontBold.flagDefined || attr->fontItalic.flagDefined || attr->fontUnderline.flagDefined) {
		sl_real fontSize;
		if (attr->finalFontSize.flagDefined) {
			fontSize = _getDimensionValue(attr->finalFontSize);
		} else {
			fontSize = UI::getDefaultFontSize();
		}
		if (attr->finalFontFamily.flagDefined) {
			view->setFont(_getStringValue(attr->finalFontFamily), fontSize, attr->finalFontBold, attr->finalFontItalic, attr->finalFontUnderline, sl_false);
		} else {
			view->setFontAttributes(fontSize, attr->finalFontBold, attr->finalFontItalic, attr->finalFontUnderline, sl_false);
		}
	}
	
	if (attr->scrollBars.horizontalScrollBar) {
		if (attr->scrollBars.verticalScrollBar) {
			view->createScrollBars(sl_false);
		} else {
			view->createHorizontalScrollBar(sl_false);
		}
	} else {
		if (attr->scrollBars.verticalScrollBar) {
			view->createVerticalScrollBar(sl_false);
		}
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByMouse, setContentScrollingByMouse, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByTouch, setContentScrollingByTouch, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByMouseWheel, setContentScrollingByMouseWheel, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByKeyboard, setContentScrollingByKeyboard, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, multiTouch, setMultiTouchMode, _get)
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, childInstances, setCreatingChildInstances, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, doubleBuffering, setDoubleBuffering, _get)
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceWindowAttributes(SAppLayoutResource* item)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutWindowAttributes> attr = new SAppLayoutWindowAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, menu)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, title)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, left)
	if (!(attr->left.checkForWindow())) {
		LOG_ERROR_LAYOUT_ATTR(left)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, top)
	if (!(attr->top.checkForWindow())) {
		LOG_ERROR_LAYOUT_ATTR(top)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, width)
	if (!(attr->width.checkForWindowSize())) {
		LOG_ERROR_LAYOUT_ATTR(width)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, height)
	if (!(attr->height.checkForWindowSize())) {
		LOG_ERROR_LAYOUT_ATTR(height)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, minimized)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, maximized)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, visible)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, alwaysOnTop)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, closeButton)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, minimizeButton)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, maximizeButton)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, resizable)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, alpha)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, transparent)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, modal)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, dialog)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, borderless)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, titleBar)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fullScreen)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, centerScreen)
	
	item->windowAttrs = attr;
	
	if (!(_parseLayoutResourceRootViewAttributes(item))) {
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppWindowAttributes(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDefine)
{
	SAppLayoutWindowAttributes* attr = item->windowAttrs.ptr;
	
	String strTab = "\t\t\t";
	
	if (!(_checkMenuValueAvailable(attr->menu, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, menu, setMenu, sbDefine)
	if (!(_checkStringValueAvailable(attr->title, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, title, setTitle, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, left, setX, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, top, setY, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, width, setWidth, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, height, setHeight, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, minimized, setMinimized, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, maximized, setMaximized, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, visible, setVisible, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, alwaysOnTop, setAlwaysOnTop, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, closeButton, setCloseButtonEnabled, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, minimizeButton, setMinimizeButtonEnabled, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, maximizeButton, setMaximizeButtonEnabled, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, resizable, setResizable, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, alpha, setAlpha, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, transparent, setTransparent, sbDefine)
	
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, modal, setModal, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, dialog, setDialog, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, borderless, setBorderless, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, titleBar, setTitleBarVisible, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, fullScreen, setFullScreenOnCreate, sbDefine)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, centerScreen, setCenterScreenOnCreate, sbDefine)
	
	if (!(_generateLayoutsCppRootViewAttributes(item, sbDefine))) {
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetWindowAttributes(Window* view, SAppLayoutResourceItem* item)
{
	SAppLayoutWindowAttributes* attr = item->windowAttrs.ptr;
	sl_bool flagOnLayout = sl_false;
	
	if (!(_checkMenuValueAvailable(attr->menu, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, menu, setMenu, _getMenu)
	if (!(_checkStringValueAvailable(attr->title, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, title, setTitle, _getString)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, left, setX, _getDimension)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, top, setY, _getDimension)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, width, setWidth, _getDimension)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, height, setHeight, _getDimension)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, minimized, setMinimized, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, maximized, setMaximized, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, visible, setVisible, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, alwaysOnTop, setAlwaysOnTop, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, closeButton, setCloseButtonEnabled, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, minimizeButton, setMinimizeButtonEnabled, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, maximizeButton, setMaximizeButtonEnabled, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, resizable, setResizable, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, alpha, setAlpha, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, transparent, setTransparent, _get)
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, modal, setModal, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, dialog, setDialog, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, borderless, setBorderless, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, titleBar, setTitleBarVisible, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, fullScreen, setFullScreenOnCreate, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, centerScreen, setCenterScreenOnCreate, _get)

	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceViewAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutViewAttributes> attr = new SAppLayoutViewAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, _id)
	
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, width)
		if (parent) {
			if (!(attr->width.checkSize())) {
				LOG_ERROR_LAYOUT_ATTR(width)
				return sl_false;
			}
		} else {
			if (!(attr->width.checkForRootViewSize())) {
				LOG_ERROR_LAYOUT_ATTR(width)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, height)
		if (parent) {
			if (!(attr->height.checkSize())) {
				LOG_ERROR_LAYOUT_ATTR(height)
				return sl_false;
			}
		} else {
			if (!(attr->height.checkForRootViewSize())) {
				LOG_ERROR_LAYOUT_ATTR(height)
				return sl_false;
			}
		}
	}
	
	attr->leftMode = PositionMode::Fixed;
	attr->topMode = PositionMode::Fixed;
	attr->rightMode = PositionMode::Fixed;
	attr->bottomMode = PositionMode::Fixed;
	
	{
		SAppAlignLayoutValue alignLeft;
		PARSE_AND_CHECK_LAYOUT_ATTR(, alignLeft)
		if (alignLeft.flagDefined) {
			if (alignLeft.flagAlignParent) {
				attr->leftMode = PositionMode::ParentEdge;
			} else {
				if (parent && layout->itemsByName.contains(alignLeft.referingView)) {
					attr->leftMode = PositionMode::OtherStart;
					attr->leftReferingView = alignLeft.referingView;
				} else {
					LOG_ERROR_LAYOUT_ATTR(alignLeft)
					return sl_false;
				}
			}
		}
	}
	{
		SAppNameValue toRightOf;
		PARSE_AND_CHECK_LAYOUT_ATTR(, toRightOf)
		if (toRightOf.flagDefined) {
			if (parent && layout->itemsByName.contains(toRightOf.value)) {
				attr->leftMode = PositionMode::OtherEnd;
				attr->leftReferingView = toRightOf.value;
			} else {
				LOG_ERROR_LAYOUT_ATTR(toRightOf)
				return sl_false;
			}
		}
	}
	{
		SAppAlignLayoutValue alignTop;
		PARSE_AND_CHECK_LAYOUT_ATTR(, alignTop)
		if (alignTop.flagDefined) {
			if (alignTop.flagAlignParent) {
				attr->topMode = PositionMode::ParentEdge;
			} else {
				if (parent && layout->itemsByName.contains(alignTop.referingView)) {
					attr->topMode = PositionMode::OtherStart;
					attr->topReferingView = alignTop.referingView;
				} else {
					LOG_ERROR_LAYOUT_ATTR(alignTop)
					return sl_false;
				}
			}
		}
	}
	{
		SAppNameValue below;
		PARSE_AND_CHECK_LAYOUT_ATTR(, below)
		if (below.flagDefined) {
			if (parent && layout->itemsByName.contains(below.value)) {
				attr->topMode = PositionMode::OtherEnd;
				attr->topReferingView = below.value;
			} else {
				LOG_ERROR_LAYOUT_ATTR(below)
				return sl_false;
			}
		}
	}
	{
		SAppAlignLayoutValue alignRight;
		PARSE_AND_CHECK_LAYOUT_ATTR(, alignRight)
		if (alignRight.flagDefined) {
			if (alignRight.flagAlignParent) {
				attr->rightMode = PositionMode::ParentEdge;
			} else {
				if (parent && layout->itemsByName.contains(alignRight.referingView)) {
					attr->rightMode = PositionMode::OtherEnd;
					attr->rightReferingView = alignRight.referingView;
				} else {
					LOG_ERROR_LAYOUT_ATTR(alignRight)
					return sl_false;
				}
			}
		}
	}
	{
		SAppNameValue toLeftOf;
		PARSE_AND_CHECK_LAYOUT_ATTR(, toLeftOf)
		if (toLeftOf.flagDefined) {
			if (parent && layout->itemsByName.contains(toLeftOf.value)) {
				attr->rightMode = PositionMode::OtherStart;
				attr->rightReferingView = toLeftOf.value;
			} else {
				LOG_ERROR_LAYOUT_ATTR(toLeftOf)
				return sl_false;
			}
		}
	}
	{
		SAppAlignLayoutValue alignBottom;
		PARSE_AND_CHECK_LAYOUT_ATTR(, alignBottom)
		if (alignBottom.flagDefined) {
			if (alignBottom.flagAlignParent) {
				attr->bottomMode = PositionMode::ParentEdge;
			} else {
				if (parent && layout->itemsByName.contains(alignBottom.referingView)) {
					attr->bottomMode = PositionMode::OtherEnd;
					attr->bottomReferingView = alignBottom.referingView;
				} else {
					LOG_ERROR_LAYOUT_ATTR(alignBottom)
					return sl_false;
				}
			}
		}
	}
	{
		SAppNameValue above;
		PARSE_AND_CHECK_LAYOUT_ATTR(, above)
		if (above.flagDefined) {
			if (parent && layout->itemsByName.contains(above.value)) {
				attr->bottomMode = PositionMode::OtherStart;
				attr->bottomReferingView = above.value;
			} else {
				LOG_ERROR_LAYOUT_ATTR(above)
				return sl_false;
			}
		}
	}
	{
		SAppAlignLayoutValue centerHorizontal;
		PARSE_AND_CHECK_LAYOUT_ATTR(, centerHorizontal)
		if (centerHorizontal.flagDefined) {
			if (centerHorizontal.flagAlignParent) {
				attr->leftMode = PositionMode::CenterInParent;
			} else {
				if (parent && layout->itemsByName.contains(centerHorizontal.referingView)) {
					attr->leftMode = PositionMode::CenterInOther;
					attr->leftReferingView = centerHorizontal.referingView;
				} else {
					LOG_ERROR_LAYOUT_ATTR(centerHorizontal)
					return sl_false;
				}
			}
		}
	}
	{
		SAppAlignLayoutValue centerVertical;
		PARSE_AND_CHECK_LAYOUT_ATTR(, centerVertical)
		if (centerVertical.flagDefined) {
			if (centerVertical.flagAlignParent) {
				attr->topMode = PositionMode::CenterInParent;
			} else {
				if (parent && layout->itemsByName.contains(centerVertical.referingView)) {
					attr->topMode = PositionMode::CenterInOther;
					attr->topReferingView = centerVertical.referingView;
				} else {
					LOG_ERROR_LAYOUT_ATTR(centerVertical)
					return sl_false;
				}
			}
		}
	}
	{
		SAppAlignLayoutValue alignCenter;
		PARSE_AND_CHECK_LAYOUT_ATTR(, alignCenter)
		if (alignCenter.flagDefined) {
			if (alignCenter.flagAlignParent) {
				attr->leftMode = PositionMode::CenterInParent;
				attr->topMode = PositionMode::CenterInParent;
			} else {
				if (parent && layout->itemsByName.contains(alignCenter.referingView)) {
					attr->leftMode = PositionMode::CenterInOther;
					attr->topMode = PositionMode::CenterInOther;
					attr->leftReferingView = alignCenter.referingView;
				} else {
					LOG_ERROR_LAYOUT_ATTR(alignCenter)
					return sl_false;
				}
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, left)
		if (parent) {
			if (!(attr->left.checkPosition())) {
				LOG_ERROR_LAYOUT_ATTR(left)
				return sl_false;
			}
		} else {
			if (!(attr->left.checkForRootViewPosition())) {
				LOG_ERROR_LAYOUT_ATTR(left)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, top)
		if (parent) {
			if (!(attr->top.checkPosition())) {
				LOG_ERROR_LAYOUT_ATTR(top)
				return sl_false;
			}
		} else {
			if (!(attr->top.checkForRootViewPosition())) {
				LOG_ERROR_LAYOUT_ATTR(top)
				return sl_false;
			}
		}
	}
	
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, marginLeft)
		if (parent) {
			if (!(attr->marginLeft.checkMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginLeft)
				return sl_false;
			}
		} else {
			if (!(attr->marginLeft.checkForRootViewMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginLeft)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, marginTop)
		if (parent) {
			if (!(attr->marginTop.checkMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginTop)
				return sl_false;
			}
		} else {
			if (!(attr->marginTop.checkForRootViewMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginTop)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, marginRight)
		if (parent) {
			if (!(attr->marginRight.checkMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginRight)
				return sl_false;
			}
		} else {
			if (!(attr->marginRight.checkForRootViewMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginRight)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, marginBottom)
		if (parent) {
			if (!(attr->marginBottom.checkMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginBottom)
				return sl_false;
			}
		} else {
			if (!(attr->marginBottom.checkForRootViewMargin())) {
				LOG_ERROR_LAYOUT_ATTR(marginBottom)
				return sl_false;
			}
		}
	}
	{
		SAppDimensionValue margin;
		PARSE_AND_CHECK_LAYOUT_ATTR(, margin)
		if (parent) {
			if (!(margin.checkMargin())) {
				LOG_ERROR_LAYOUT_ATTR(margin)
				return sl_false;
			}
		} else {
			if (!(margin.checkForRootViewMargin())) {
				LOG_ERROR_LAYOUT_ATTR(margin)
				return sl_false;
			}
		}
		if (margin.flagDefined) {
			if (!(attr->marginLeft.flagDefined)) {
				attr->marginLeft = margin;
			}
			if (!(attr->marginTop.flagDefined)) {
				attr->marginTop = margin;
			}
			if (!(attr->marginRight.flagDefined)) {
				attr->marginRight = margin;
			}
			if (!(attr->marginBottom.flagDefined)) {
				attr->marginBottom = margin;
			}
		}
	}
	
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingLeft)
		if (parent) {
			if (!(attr->paddingLeft.checkPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingLeft)
				return sl_false;
			}
		} else {
			if (!(attr->paddingLeft.checkForRootViewPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingLeft)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingTop)
		if (parent) {
			if (!(attr->paddingTop.checkPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingTop)
				return sl_false;
			}
		} else {
			if (!(attr->paddingTop.checkForRootViewPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingTop)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingRight)
		if (parent) {
			if (!(attr->paddingRight.checkPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingRight)
				return sl_false;
			}
		} else {
			if (!(attr->paddingRight.checkForRootViewPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingRight)
				return sl_false;
			}
		}
	}
	{
		PARSE_AND_CHECK_LAYOUT_ATTR(attr->, paddingBottom)
		if (parent) {
			if (!(attr->paddingBottom.checkPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingBottom)
				return sl_false;
			}
		} else {
			if (!(attr->paddingBottom.checkForRootViewPosition())) {
				LOG_ERROR_LAYOUT_ATTR(paddingBottom)
				return sl_false;
			}
		}
	}
	{
		SAppDimensionValue padding;
		PARSE_AND_CHECK_LAYOUT_ATTR(, padding)
		if (parent) {
			if (!(padding.checkPosition())) {
				LOG_ERROR_LAYOUT_ATTR(padding)
				return sl_false;
			}
		} else {
			if (!(padding.checkForRootViewPosition())) {
				LOG_ERROR_LAYOUT_ATTR(padding)
				return sl_false;
			}
		}
		if (padding.flagDefined) {
			if (!(attr->paddingLeft.flagDefined)) {
				attr->paddingLeft = padding;
			}
			if (!(attr->paddingTop.flagDefined)) {
				attr->paddingTop = padding;
			}
			if (!(attr->paddingRight.flagDefined)) {
				attr->paddingRight = padding;
			}
			if (!(attr->paddingBottom.flagDefined)) {
				attr->paddingBottom = padding;
			}
		}
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, visibility)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, enabled)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, opaque)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, occurringClick)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, background)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, backgroundScale)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, backgroundAlign)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, backgroundColor)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, borderWidth)
	if (parent) {
		if (!(attr->borderWidth.checkPosition())) {
			LOG_ERROR_LAYOUT_ATTR(borderWidth)
			return sl_false;
		}
	} else {
		if (!(attr->borderWidth.checkForRootViewPosition())) {
			LOG_ERROR_LAYOUT_ATTR(borderWidth)
			return sl_false;
		}
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, borderColor)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, borderStyle)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontFamily)
	if (attr->fontFamily.flagDefined) {
		attr->finalFontFamily = attr->fontFamily;
	} else {
		if (parent) {
			attr->finalFontFamily = parent->viewAttrs->finalFontFamily;
		}
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontSize)
	if (parent) {
		if (!(attr->fontSize.checkPosition())) {
			LOG_ERROR_LAYOUT_ATTR(fontSize)
			return sl_false;
		}
	} else {
		if (!(attr->fontSize.checkForRootViewPosition())) {
			LOG_ERROR_LAYOUT_ATTR(fontSize)
			return sl_false;
		}
	}
	if (attr->fontSize.flagDefined) {
		attr->finalFontSize = attr->fontSize;
	} else {
		if (parent) {
			attr->finalFontSize = parent->viewAttrs->finalFontSize;
		}
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontBold)
	if (attr->fontBold.flagDefined) {
		attr->finalFontBold = attr->fontBold.value;
	} else {
		if (parent) {
			attr->finalFontBold = parent->viewAttrs->finalFontBold;
		}
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontItalic)
	if (attr->fontItalic.flagDefined) {
		attr->finalFontItalic = attr->fontItalic.value;
	} else {
		if (parent) {
			attr->finalFontItalic = parent->viewAttrs->finalFontItalic;
		}
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, fontUnderline)
	if (attr->fontUnderline.flagDefined) {
		attr->finalFontUnderline = attr->fontUnderline.value;
	} else {
		if (parent) {
			attr->finalFontUnderline = parent->viewAttrs->finalFontUnderline;
		}
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollBars)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByMouse)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByTouch)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByMouseWheel)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrollingByKeyboard)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, multiTouch)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, tabStop)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, instance)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, childInstances)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, nativeWidget)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, doubleBuffering)
	
	item->viewAttrs = attr;
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppViewAttributes(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout)
{
	SAppLayoutViewAttributes* attr = item->viewAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_checkStringValueAvailable(attr->_id, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, _id, setId, sbDefineInit)
	
	if (attr->width.flagDefined) {
		if (attr->width.unit == SAppDimensionValue::FILL) {
			sbDefineInit.add(String::format("%s%s->setWidthFilling(%ff, sl_false);%n", strTab, name, attr->width.amount));
		} else if (attr->width.unit == SAppDimensionValue::WRAP) {
			sbDefineInit.add(String::format("%s%s->setWidthWrapping(sl_false);%n", strTab, name));
		} else if (attr->width.unit == SAppDimensionValue::WEIGHT) {
			sbDefineInit.add(String::format("%s%s->setWidthWeight(%ff, sl_false);%n", strTab, name, attr->width.amount));
		} else {
			GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, width, setWidth, sbDefineInit, sbDefineLayout)
		}
	}
	if (attr->height.flagDefined) {
		if (attr->height.unit == SAppDimensionValue::FILL) {
			sbDefineInit.add(String::format("%s%s->setHeightFilling(%ff, sl_false);%n", strTab, name, attr->height.amount));
		} else if (attr->height.unit == SAppDimensionValue::WRAP) {
			sbDefineInit.add(String::format("%s%s->setHeightWrapping(sl_false);%n", strTab, name));
		} else if (attr->height.unit == SAppDimensionValue::WEIGHT) {
			sbDefineInit.add(String::format("%s%s->setHeightWeight(%ff, sl_false);%n", strTab, name, attr->height.amount));
		} else {
			GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, height, setHeight, sbDefineInit, sbDefineLayout)
		}
	}

	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, left, setX, sbDefineInit, sbDefineLayout)
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, top, setY, sbDefineInit, sbDefineLayout)
	
	if (attr->leftMode == PositionMode::CenterInParent) {
		sbDefineInit.add(String::format("%s%s->setCenterHorizontal(sl_false);%n", strTab, name, attr->leftReferingView));
	} else if (attr->leftMode == PositionMode::CenterInOther) {
		sbDefineInit.add(String::format("%s%s->setAlignCenterHorizontal(%s, sl_false);%n", strTab, name, attr->leftReferingView));
	} else {
		if (attr->leftMode == PositionMode::ParentEdge) {
			sbDefineInit.add(String::format("%s%s->setAlignParentLeft(sl_false);%n", strTab, name));
		} else if (attr->leftMode == PositionMode::OtherStart) {
			sbDefineInit.add(String::format("%s%s->setAlignLeft(%s, sl_false);%n", strTab, name, attr->leftReferingView));
		} else if (attr->leftMode == PositionMode::OtherEnd) {
			sbDefineInit.add(String::format("%s%s->setRightOf(%s, sl_false);%n", strTab, name, attr->leftReferingView));
		}
		if (attr->rightMode == PositionMode::ParentEdge) {
			sbDefineInit.add(String::format("%s%s->setAlignParentRight(sl_false);%n", strTab, name));
		} else if (attr->rightMode == PositionMode::OtherStart) {
			sbDefineInit.add(String::format("%s%s->setLeftOf(%s, sl_false);%n", strTab, name, attr->rightReferingView));
		} else if (attr->rightMode == PositionMode::OtherEnd) {
			sbDefineInit.add(String::format("%s%s->setAlignRight(%s, sl_false);%n", strTab, name, attr->rightReferingView));
		}
	}
	
	if (attr->topMode == PositionMode::CenterInParent) {
		sbDefineInit.add(String::format("%s%s->setCenterVertical(sl_false);%n", strTab, name, attr->topReferingView));
	} else if (attr->topMode == PositionMode::CenterInOther) {
		sbDefineInit.add(String::format("%s%s->setAlignCenterVertical(%s, sl_false);%n", strTab, name, attr->topReferingView));
	} else {
		if (attr->topMode == PositionMode::ParentEdge) {
			sbDefineInit.add(String::format("%s%s->setAlignParentTop(sl_false);%n", strTab, name));
		} else if (attr->topMode == PositionMode::OtherStart) {
			sbDefineInit.add(String::format("%s%s->setAlignTop(%s, sl_false);%n", strTab, name, attr->topReferingView));
		} else if (attr->topMode == PositionMode::OtherEnd) {
			sbDefineInit.add(String::format("%s%s->setBelow(%s, sl_false);%n", strTab, name, attr->topReferingView));
		}
		if (attr->bottomMode == PositionMode::ParentEdge) {
			sbDefineInit.add(String::format("%s%s->setAlignParentBottom(sl_false);%n", strTab, name));
		} else if (attr->bottomMode == PositionMode::OtherStart) {
			sbDefineInit.add(String::format("%s%s->setAbove(%s, sl_false);%n", strTab, name, attr->bottomReferingView));
		} else if (attr->bottomMode == PositionMode::OtherEnd) {
			sbDefineInit.add(String::format("%s%s->setAlignBottom(%s, sl_false);%n", strTab, name, attr->bottomReferingView));
		}
	}
	
	if (attr->marginLeft.flagDefined) {
		if (attr->marginLeft.unit == SAppDimensionValue::WEIGHT) {
			sbDefineInit.add(String::format("%s%s->setRelativeMarginLeft(%ff, sl_false);%n", strTab, name, attr->marginLeft.amount));
		} else {
			GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, marginLeft, setMarginLeft, sbDefineInit, sbDefineLayout)
		}
	}
	if (attr->marginTop.flagDefined) {
		if (attr->marginTop.unit == SAppDimensionValue::WEIGHT) {
			sbDefineInit.add(String::format("%s%s->setRelativeMarginTop(%ff, sl_false);%n", strTab, name, attr->marginTop.amount));
		} else {
			GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, marginTop, setMarginTop, sbDefineInit, sbDefineLayout)
		}
	}
	if (attr->marginRight.flagDefined) {
		if (attr->marginRight.unit == SAppDimensionValue::WEIGHT) {
			sbDefineInit.add(String::format("%s%s->setRelativeMarginRight(%ff, sl_false);%n", strTab, name, attr->marginRight.amount));
		} else {
			GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, marginRight, setMarginRight, sbDefineInit, sbDefineLayout)
		}
	}
	if (attr->marginBottom.flagDefined) {
		if (attr->marginBottom.unit == SAppDimensionValue::WEIGHT) {
			sbDefineInit.add(String::format("%s%s->setRelativeMarginBottom(%ff, sl_false);%n", strTab, name, attr->marginBottom.amount));
		} else {
			GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, marginBottom, setMarginBottom, sbDefineInit, sbDefineLayout)
		}
	}

	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, paddingLeft, setPaddingLeft, sbDefineInit, sbDefineLayout)
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, paddingTop, setPaddingTop, sbDefineInit, sbDefineLayout)
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, paddingRight, setPaddingRight, sbDefineInit, sbDefineLayout)
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, paddingBottom, setPaddingBottom, sbDefineInit, sbDefineLayout)
	
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, visibility, setVisibility, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, enabled, setEnabled, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, opaque, setOpaque, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, occurringClick, setOccurringClick, sbDefineInit)
	
	if (!(_checkDrawableValueAvailable(attr->background, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, background, setBackground, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundScale, setBackgroundScaleMode, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundAlign, setBackgroundAlignment, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundColor, setBackgroundColor, sbDefineInit)
	if (attr->borderWidth.flagDefined) {
		if (Math::isAlmostZero(attr->borderWidth.amount)) {
			sbDefineInit.add(String::format("%s%s->setBorder(slib::Ref<slib::Pen>::null(), sl_false);%n", strTab, name));
		} else {
			if (attr->borderColor.flagDefined && attr->borderStyle.flagDefined) {
				StringBuffer* sb;
				if (attr->borderWidth.isNeededOnLayoutFunction()) {
					sb = &sbDefineLayout;
				} else {
					sb = &sbDefineInit;
				}
				sb->add(String::format("%s%s->setBorder(slib::Pen::create(%s, %s, %s), sl_false);%n", strTab, name, attr->borderStyle.getAccessString(), attr->borderWidth.getAccessString(), attr->borderColor.getAccessString()));
			} else {
				GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, borderWidth, setBorderWidth, sbDefineInit, sbDefineLayout)
				GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderColor, setBorderColor, sbDefineInit)
				GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderStyle, setBorderStyle, sbDefineInit)
			}
		}
	} else {
		GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderColor, setBorderColor, sbDefineInit)
		GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderStyle, setBorderStyle, sbDefineInit)
	}
	
	if (!(_checkStringValueAvailable(attr->fontFamily, item->element))) {
		return sl_false;
	}
	if (attr->fontFamily.flagDefined || attr->fontSize.flagDefined || attr->fontBold.flagDefined || attr->fontItalic.flagDefined || attr->fontUnderline.flagDefined) {
		String fontSize;
		if (attr->finalFontSize.flagDefined) {
			fontSize = attr->finalFontSize.getAccessString();
		} else {
			fontSize = "UI::getDefaultFontSize()";
		}
		StringBuffer* sb;
		if (attr->finalFontSize.isNeededOnLayoutFunction()) {
			sb = &sbDefineLayout;
		} else {
			sb = &sbDefineInit;
		}
		if (attr->finalFontFamily.flagDefined) {
			sb->add(String::format("%s%s->setFont(%s, ", strTab, name, attr->finalFontFamily.getAccessString()));
		} else {
			sb->add(String::format("%s%s->setFontAttributes(", strTab, name));
		}
		sb->add(String::format("%s, %s, %s, %s, sl_false);%n", fontSize, attr->finalFontBold?"sl_true":"sl_false", attr->finalFontItalic?"sl_true":"sl_false", attr->finalFontUnderline?"sl_true":"sl_false"));
	}
	
	if (attr->scrollBars.horizontalScrollBar) {
		if (attr->scrollBars.verticalScrollBar) {
			sbDefineInit.add(String::format("%s%s->createScrollBars(sl_false);%n", strTab, name));
		} else {
			sbDefineInit.add(String::format("%s%s->createHorizontalScrollBar(sl_false);%n", strTab, name));
		}
	} else {
		if (attr->scrollBars.verticalScrollBar) {
			sbDefineInit.add(String::format("%s%s->createVerticalScrollBar(sl_false);%n", strTab, name));
		}
	}
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByMouse, setContentScrollingByMouse, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByTouch, setContentScrollingByTouch, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByMouseWheel, setContentScrollingByMouseWheel, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, scrollingByKeyboard, setContentScrollingByKeyboard, sbDefineInit)
	
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, multiTouch, setMultiTouchMode, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, tabStop, setProcessingTabStop, sbDefineInit)
	
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, instance, setCreatingInstance, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, childInstances, setCreatingChildInstances, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR(attr->, nativeWidget, setCreatingNativeWidget, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, doubleBuffering, setDoubleBuffering, sbDefineInit)
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetViewAttributes(SAppLayoutSimulationWindow* simulator, View* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutViewAttributes* attr = item->viewAttrs.ptr;
	
	if (!(_checkStringValueAvailable(attr->_id, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, _id, setId, _getString)
	
	if (attr->width.flagDefined) {
		if (attr->width.unit == SAppDimensionValue::FILL) {
			if (!flagOnLayout) {
				view->setWidthFilling(attr->width.amount, sl_false);
			}
		} else if (attr->width.unit == SAppDimensionValue::WRAP) {
			if (!flagOnLayout) {
				view->setWidthWrapping(sl_false);
			}
		} else if (attr->width.unit == SAppDimensionValue::WEIGHT) {
			if (!flagOnLayout) {
				view->setWidthWeight(attr->width.amount, sl_false);
			}
		} else {
			SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, width, setWidth)
		}
	}
	if (attr->height.flagDefined) {
		if (attr->height.unit == SAppDimensionValue::FILL) {
			if (!flagOnLayout) {
				view->setHeightFilling(attr->height.amount, sl_false);
			}
		} else if (attr->height.unit == SAppDimensionValue::WRAP) {
			if (!flagOnLayout) {
				view->setHeightWrapping(sl_false);
			}
		} else if (attr->height.unit == SAppDimensionValue::WEIGHT) {
			if (!flagOnLayout) {
				view->setHeightWeight(attr->height.amount, sl_false);
			}
		} else {
			SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, height, setHeight)
		}
	}
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, left, setX)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, top, setY)
	
	if (attr->leftMode == PositionMode::CenterInParent) {
		if (!flagOnLayout) {
			view->setCenterHorizontal(sl_false);
		}
	} else if (attr->leftMode == PositionMode::CenterInOther) {
		if (!flagOnLayout) {
			view->setAlignCenterHorizontal(simulator->getViewByName(attr->leftReferingView), sl_false);
		}
	} else {
		if (attr->leftMode == PositionMode::ParentEdge) {
			if (!flagOnLayout) {
				view->setAlignParentLeft(sl_false);
			}
		} else if (attr->leftMode == PositionMode::OtherStart) {
			if (!flagOnLayout) {
				view->setAlignLeft(simulator->getViewByName(attr->leftReferingView), sl_false);
			}
		} else if (attr->leftMode == PositionMode::OtherEnd) {
			if (!flagOnLayout) {
				view->setRightOf(simulator->getViewByName(attr->leftReferingView), sl_false);
			}
		}
		if (attr->rightMode == PositionMode::ParentEdge) {
			if (!flagOnLayout) {
				view->setAlignParentRight(sl_false);
			}
		} else if (attr->rightMode == PositionMode::OtherStart) {
			if (!flagOnLayout) {
				view->setLeftOf(simulator->getViewByName(attr->rightReferingView), sl_false);
			}
		} else if (attr->rightMode == PositionMode::OtherEnd) {
			if (!flagOnLayout) {
				view->setAlignRight(simulator->getViewByName(attr->rightReferingView), sl_false);
			}
		}
	}
	
	if (attr->topMode == PositionMode::CenterInParent) {
		if (!flagOnLayout) {
			view->setCenterVertical(sl_false);
		}
	} else if (attr->topMode == PositionMode::CenterInOther) {
		if (!flagOnLayout) {
			view->setAlignCenterVertical(simulator->getViewByName(attr->topReferingView), sl_false);
		}
	} else {
		if (attr->topMode == PositionMode::ParentEdge) {
			if (!flagOnLayout) {
				view->setAlignParentTop(sl_false);
			}
		} else if (attr->topMode == PositionMode::OtherStart) {
			if (!flagOnLayout) {
				view->setAlignTop(simulator->getViewByName(attr->topReferingView), sl_false);
			}
		} else if (attr->topMode == PositionMode::OtherEnd) {
			if (!flagOnLayout) {
				view->setBelow(simulator->getViewByName(attr->topReferingView), sl_false);
			}
		}
		if (attr->bottomMode == PositionMode::ParentEdge) {
			if (!flagOnLayout) {
				view->setAlignParentBottom(sl_false);
			}
		} else if (attr->bottomMode == PositionMode::OtherStart) {
			if (!flagOnLayout) {
				view->setAbove(simulator->getViewByName(attr->bottomReferingView), sl_false);
			}
		} else if (attr->bottomMode == PositionMode::OtherEnd) {
			if (!flagOnLayout) {
				view->setAlignBottom(simulator->getViewByName(attr->bottomReferingView), sl_false);
			}
		}
	}
	
	if (attr->marginLeft.flagDefined) {
		if (attr->marginLeft.unit == SAppDimensionValue::WEIGHT) {
			if (!flagOnLayout) {
				view->setRelativeMarginLeft(attr->marginLeft.amount, sl_false);
			}
		} else {
			SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, marginLeft, setMarginLeft)
		}
	}
	if (attr->marginTop.flagDefined) {
		if (attr->marginTop.unit == SAppDimensionValue::WEIGHT) {
			if (!flagOnLayout) {
				view->setRelativeMarginTop(attr->marginTop.amount, sl_false);
			}
		} else {
			SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, marginTop, setMarginTop)
		}
	}
	if (attr->marginRight.flagDefined) {
		if (attr->marginRight.unit == SAppDimensionValue::WEIGHT) {
			if (!flagOnLayout) {
				view->setRelativeMarginRight(attr->marginRight.amount, sl_false);
			}
		} else {
			SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, marginRight, setMarginRight)
		}
	}
	if (attr->marginBottom.flagDefined) {
		if (attr->marginBottom.unit == SAppDimensionValue::WEIGHT) {
			if (!flagOnLayout) {
				view->setRelativeMarginBottom(attr->marginBottom.amount, sl_false);
			}
		} else {
			SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, marginBottom, setMarginBottom)
		}
	}
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, paddingLeft, setPaddingLeft)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, paddingTop, setPaddingTop)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, paddingRight, setPaddingRight)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, paddingBottom, setPaddingBottom)
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, visibility, setVisibility, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, enabled, setEnabled, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, opaque, setOpaque, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, occurringClick, setOccurringClick, _get)
	
	if (!(_checkDrawableValueAvailable(attr->background, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_DRAWABLE(attr->, background, setBackground)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundScale, setBackgroundScaleMode, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundAlign, setBackgroundAlignment, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, backgroundColor, setBackgroundColor, _get)
	if (attr->borderWidth.flagDefined) {
		if (Math::isAlmostZero(attr->borderWidth.amount)) {
			if (!flagOnLayout) {
				view->setBorder(Ref<Pen>::null(), sl_false);
			}
		} else {
			if (attr->borderColor.flagDefined && attr->borderStyle.flagDefined) {
				if (flagOnLayout) {
					view->setBorder(Pen::create(attr->borderStyle.value, _getDimensionValue(attr->borderWidth), attr->borderColor.value), sl_false);
				}
			} else {
				SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, borderWidth, setBorderWidth)
				SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderColor, setBorderColor, _get)
				SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderStyle, setBorderStyle, _get)
			}
		}
	} else {
		SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderColor, setBorderColor, _get)
		SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, borderStyle, setBorderStyle, _get)
	}
	
	if (flagOnLayout) {
		if (!(_checkStringValueAvailable(attr->fontFamily, item->element))) {
			return sl_false;
		}
		if (attr->fontFamily.flagDefined || attr->fontSize.flagDefined || attr->fontBold.flagDefined || attr->fontItalic.flagDefined || attr->fontUnderline.flagDefined) {
			sl_real fontSize;
			if (attr->finalFontSize.flagDefined) {
				fontSize = _getDimensionValue(attr->finalFontSize);
			} else {
				fontSize = UI::getDefaultFontSize();
			}
			if (attr->finalFontFamily.flagDefined) {
				view->setFont(_getStringValue(attr->finalFontFamily), fontSize, attr->finalFontBold, attr->finalFontItalic, attr->finalFontUnderline, sl_false);
			} else {
				view->setFontAttributes(fontSize, attr->finalFontBold, attr->finalFontItalic, attr->finalFontUnderline, sl_false);
			}
		}
	}
	
	if (attr->scrollBars.horizontalScrollBar) {
		if (attr->scrollBars.verticalScrollBar) {
			view->createScrollBars(sl_false);
		} else {
			view->createHorizontalScrollBar(sl_false);
		}
	} else {
		if (attr->scrollBars.verticalScrollBar) {
			view->createVerticalScrollBar(sl_false);
		}
	}

	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByMouse, setContentScrollingByMouse, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByTouch, setContentScrollingByTouch, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByMouseWheel, setContentScrollingByMouseWheel, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, scrollingByKeyboard, setContentScrollingByKeyboard, _get)
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, multiTouch, setMultiTouchMode, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, tabStop, setProcessingTabStop, _get)
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, instance, setCreatingInstance, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, childInstances, setCreatingChildInstances, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR(attr->, nativeWidget, setCreatingNativeWidget, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, doubleBuffering, setDoubleBuffering, _get)
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceButtonAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutButtonAttributes> attr = new SAppLayoutButtonAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->buttonAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, text)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, defaultButton)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textColor)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, icon)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconWidth)
	if (!(attr->iconWidth.checkScalarSize())) {
		LOG_ERROR_LAYOUT_ATTR(iconWidth)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconHeight)
	if (!(attr->iconHeight.checkScalarSize())) {
		LOG_ERROR_LAYOUT_ATTR(iconHeight)
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, gravity)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconAlign)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textAlign)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textBeforeIcon)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, orientation)
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconMarginLeft)
	if (!(attr->iconMarginLeft.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(iconMarginLeft)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconMarginTop)
	if (!(attr->iconMarginTop.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(iconMarginTop)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconMarginRight)
	if (!(attr->iconMarginRight.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(iconMarginRight)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconMarginBottom)
	if (!(attr->iconMarginBottom.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(iconMarginBottom)
		return sl_false;
	}
	SAppDimensionValue iconMargin;
	PARSE_AND_CHECK_LAYOUT_ATTR(, iconMargin)
	if (!(iconMargin.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(iconMargin)
		return sl_false;
	}
	if (iconMargin.flagDefined) {
		if (!(attr->iconMarginLeft.flagDefined)) {
			attr->iconMarginLeft = iconMargin;
		}
		if (!(attr->iconMarginTop.flagDefined)) {
			attr->iconMarginTop = iconMargin;
		}
		if (!(attr->iconMarginRight.flagDefined)) {
			attr->iconMarginRight = iconMargin;
		}
		if (!(attr->iconMarginBottom.flagDefined)) {
			attr->iconMarginBottom = iconMargin;
		}
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textMarginLeft)
	if (!(attr->textMarginLeft.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(textMarginLeft)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textMarginTop)
	if (!(attr->textMarginTop.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(textMarginTop)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textMarginRight)
	if (!(attr->textMarginRight.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(textMarginRight)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textMarginBottom)
	if (!(attr->textMarginBottom.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(textMarginBottom)
		return sl_false;
	}
	SAppDimensionValue textMargin;
	PARSE_AND_CHECK_LAYOUT_ATTR(, textMargin)
	if (!(textMargin.checkPosition())) {
		LOG_ERROR_LAYOUT_ATTR(textMargin)
		return sl_false;
	}
	if (textMargin.flagDefined) {
		if (!(attr->textMarginLeft.flagDefined)) {
			attr->textMarginLeft = textMargin;
		}
		if (!(attr->textMarginTop.flagDefined)) {
			attr->textMarginTop = textMargin;
		}
		if (!(attr->textMarginRight.flagDefined)) {
			attr->textMarginRight = textMargin;
		}
		if (!(attr->textMarginBottom.flagDefined)) {
			attr->textMarginBottom = textMargin;
		}
	}
	
	{
		String strStates[] = {"Normal", "Hover", "Down", "Disabled"};
		for (sl_uint32 i = 0; i < SLIB_SAPP_LAYOUT_BUTTON_CATEGORY_MAX; i++) {
			SAppLayoutButtonCategory& category = attr->categories[i];
			for (sl_uint32 k = 0; k < (sl_uint32)(ButtonState::Count); k++) {
				String suffix;
				if (i > 0) {
					suffix = String::format("%s%d", strStates[k], i);
				} else {
					suffix = strStates[k];
				}
				String _strTextColor = item->getXmlAttribute("textColor" + suffix);
				if (!(category.textColor[k].parse(_strTextColor))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("textColor" + suffix, _strTextColor));
					return sl_false;
				}
				String _strIcon = item->getXmlAttribute("icon" + suffix);
				if (!(category.icon[k].parse(_strIcon))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("icon" + suffix, _strIcon));
					return sl_false;
				}
				String _strBackgroundColor = item->getXmlAttribute("backgroundColor" + suffix);
				if (!(category.backgroundColor[k].parse(_strBackgroundColor))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("backgroundColor" + suffix, _strBackgroundColor));
					return sl_false;
				}
				String _strBackground = item->getXmlAttribute("background" + suffix);
				if (!(category.background[k].parse(_strBackground))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("background" + suffix, _strBackground));
					return sl_false;
				}
				String _strBorderStyle = item->getXmlAttribute("borderStyle" + suffix);
				if (!(category.borderStyle[k].parse(_strBorderStyle))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderStyle" + suffix, _strBorderStyle));
					return sl_false;
				}
				String _strBorderWidth = item->getXmlAttribute("borderWidth" + suffix);
				if (!(category.borderWidth[k].parse(_strBorderWidth))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderWidth" + suffix, _strBorderWidth));
					return sl_false;
				}
				if (!(category.borderWidth[k].checkScalarSize())) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderWidth" + suffix, _strBorderWidth));
					return sl_false;
				}
				String _strBorderColor = item->getXmlAttribute("borderColor" + suffix);
				if (!(category.borderColor[k].parse(_strBorderColor))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderColor" + suffix, _strBorderColor));
					return sl_false;
				}
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppButton(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutButtonAttributes* attr = item->buttonAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	sl_bool flagRequireNoNative = sl_false;

	if (!(_checkStringValueAvailable(attr->text, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, text, setText, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, defaultButton, setDefaultButton, sbDefineInit)
	
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, sbDefineInit)
	if (attr->textColor.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (!(_checkDrawableValueAvailable(attr->icon, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, icon, setIcon, sbDefineInit)
	if (attr->icon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconWidth, setIconWidth, sbDefineInit, sbDefineLayout)
	if (attr->iconWidth.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconHeight, setIconHeight, sbDefineInit, sbDefineLayout)
	if (attr->iconHeight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, sbDefineInit)
	if (attr->gravity.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, iconAlign, setIconAlignment, sbDefineInit)
	if (attr->iconAlign.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, textAlign, setTextAlignment, sbDefineInit)
	if (attr->textAlign.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, textBeforeIcon, setTextBeforeIcon, sbDefineInit)
	if (attr->textBeforeIcon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, orientation, setLayoutOrientation, sbDefineInit)
	if (attr->orientation.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconMarginLeft, setIconMarginLeft, sbDefineInit, sbDefineLayout)
	if (attr->iconMarginLeft.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconMarginTop, setIconMarginTop, sbDefineInit, sbDefineLayout)
	if (attr->iconMarginTop.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconMarginRight, setIconMarginRight, sbDefineInit, sbDefineLayout)
	if (attr->iconMarginRight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconMarginBottom, setIconMarginBottom, sbDefineInit, sbDefineLayout)
	if (attr->iconMarginBottom.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, textMarginLeft, setTextMarginLeft, sbDefineInit, sbDefineLayout)
	if (attr->textMarginLeft.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, textMarginTop, setTextMarginTop, sbDefineInit, sbDefineLayout)
	if (attr->textMarginTop.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, textMarginRight, setTextMarginRight, sbDefineInit, sbDefineLayout)
	if (attr->textMarginRight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, textMarginBottom, setTextMarginBottom, sbDefineInit, sbDefineLayout)
	if (attr->textMarginBottom.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (attr->textColor.flagDefined) {
		sbDefineInit.add(String::format("%s%s->resetStateTextColors(sl_false);%n", strTab, name));
	}
	if (attr->icon.flagDefined) {
		sbDefineInit.add(String::format("%s%s->resetStateIcons(sl_false);%n", strTab, name));
	}
	if (item->viewAttrs->backgroundColor.flagDefined) {
		sbDefineInit.add(String::format("%s%s->resetStateBackgroundColors(sl_false);%n", strTab, name));
	}
	if (item->viewAttrs->background.flagDefined) {
		sbDefineInit.add(String::format("%s%s->resetStateBackgrounds(sl_false);%n", strTab, name));
	}
	if (item->viewAttrs->borderWidth.flagDefined || item->viewAttrs->borderColor.flagDefined || item->viewAttrs->borderStyle.flagDefined) {
		sbDefineInit.add(String::format("%s%s->resetStateBorders(sl_false);%n", strTab, name));
	}

	{
		String strStates[] = {"Normal", "Hover", "Down", "Disabled"};
		for (sl_uint32 i = 0; i < SLIB_SAPP_LAYOUT_BUTTON_CATEGORY_MAX; i++) {
			SAppLayoutButtonCategory& category = attr->categories[i];
			for (sl_uint32 k = 0; k < (sl_uint32)(ButtonState::Count); k++) {
				if (category.textColor[k].flagDefined) {
					sbDefineInit.add(String::format("%s%s->setTextColor(%s, slib::ButtonState::%s, %d, sl_false);%n", strTab, name, category.textColor[k].getAccessString(), strStates[k], i));
					flagRequireNoNative = sl_true;
				}
				if (category.icon[k].flagDefined) {
					if (!(_checkDrawableValueAvailable(category.icon[k], item->element))) {
						return sl_false;
					}
					sbDefineInit.add(String::format("%s%s->setIcon(%s, slib::ButtonState::%s, %d, sl_false);%n", strTab, name, category.icon[k].getAccessString(), strStates[k], i));
					flagRequireNoNative = sl_true;
				}
				if (category.backgroundColor[k].flagDefined) {
					sbDefineInit.add(String::format("%s%s->setBackgroundColor(%s, slib::ButtonState::%s, %d, sl_false);%n", strTab, name, category.backgroundColor[k].getAccessString(), strStates[k], i));
					flagRequireNoNative = sl_true;
				}
				if (category.background[k].flagDefined) {
					if (!(_checkDrawableValueAvailable(category.background[k], item->element))) {
						return sl_false;
					}
					sbDefineInit.add(String::format("%s%s->setBackground(%s, slib::ButtonState::%s, %d, sl_false);%n", strTab, name, category.background[k].getAccessString(), strStates[k], i));
					flagRequireNoNative = sl_true;
				}
				if (category.borderWidth[k].flagDefined || category.borderColor[k].flagDefined || category.borderStyle[k].flagDefined) {
					if (category.borderWidth[k].flagDefined && Math::isAlmostZero(category.borderWidth[k].amount)) {
						sbDefineInit.add(String::format("%s%s->setBorder(slib::Ref<slib::Pen>::null(), slib::ButtonState::%s, %d, sl_false);%n", strTab, name, strStates[k], i));
					} else {
						StringBuffer* sb;
						if (category.borderWidth[k].isNeededOnLayoutFunction()) {
							sb = &sbDefineLayout;
						} else {
							sb = &sbDefineInit;
						}
						String _borderWidth, _borderColor, _borderStyle;
						if (category.borderWidth[k].flagDefined) {
							_borderWidth = category.borderWidth[k].getAccessString();
						} else {
							_borderWidth = "1";
						}
						if (category.borderColor[k].flagDefined) {
							_borderColor = category.borderColor[k].getAccessString();
						} else {
							_borderColor = "slib::Color::Black";
						}
						if (category.borderStyle[k].flagDefined) {
							_borderStyle = category.borderStyle[k].getAccessString();
						} else {
							_borderStyle = "slib::PenStyle::Solid";
						}
						sb->add(String::format("%s%s->setBorder(slib::Pen::create(%s, %s, %s), slib::ButtonState::%s, %d, sl_false);%n", strTab, name, _borderStyle, _borderWidth, _borderColor, strStates[k], i));
					}
					flagRequireNoNative = sl_true;
				}
			}
		}
	}
	
	if (flagRequireNoNative) {
		if (!(item->viewAttrs->nativeWidget.flagDefined)) {
			sbDefineInit.add(String::format("%s%s->setCreatingNativeWidget(sl_false);%n", strTab, name));
		}
	}
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetButtonAttributes(SAppLayoutSimulationWindow* simulator, Button* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutButtonAttributes* attr = item->buttonAttrs.ptr;
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	sl_bool flagRequireNoNative = sl_false;
	
	if (!(_checkStringValueAvailable(attr->text, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, text, setText, _getString)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, defaultButton, setDefaultButton, _get)
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, _get)
	if (attr->textColor.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (!(_checkDrawableValueAvailable(attr->icon, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_DRAWABLE(attr->, icon, setIcon)
	if (attr->icon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconWidth, setIconWidth)
	if (attr->iconWidth.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconHeight, setIconHeight)
	if (attr->iconHeight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, _get)
	if (attr->gravity.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, iconAlign, setIconAlignment, _get)
	if (attr->iconAlign.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, textAlign, setTextAlignment, _get)
	if (attr->textAlign.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, textBeforeIcon, setTextBeforeIcon, _get)
	if (attr->textBeforeIcon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, orientation, setLayoutOrientation, _get)
	if (attr->orientation.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconMarginLeft, setIconMarginLeft)
	if (attr->iconMarginLeft.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconMarginTop, setIconMarginTop)
	if (attr->iconMarginTop.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconMarginRight, setIconMarginRight)
	if (attr->iconMarginRight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconMarginBottom, setIconMarginBottom)
	if (attr->iconMarginBottom.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, textMarginLeft, setTextMarginLeft)
	if (attr->textMarginLeft.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, textMarginTop, setTextMarginTop)
	if (attr->textMarginTop.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, textMarginRight, setTextMarginRight)
	if (attr->textMarginRight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, textMarginBottom, setTextMarginBottom)
	if (attr->textMarginBottom.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (attr->textColor.flagDefined) {
		if (!flagOnLayout) {
			view->resetStateTextColors(sl_false);
		}
	}
	if (attr->icon.flagDefined) {
		if (!flagOnLayout) {
			view->resetStateIcons(sl_false);
		}
	}
	if (item->viewAttrs->backgroundColor.flagDefined) {
		if (!flagOnLayout) {
			view->resetStateBackgroundColors(sl_false);
		}
	}
	if (item->viewAttrs->background.flagDefined) {
		if (!flagOnLayout) {
			view->resetStateBackgrounds(sl_false);
		}
	}
	if (item->viewAttrs->borderWidth.flagDefined || item->viewAttrs->borderColor.flagDefined || item->viewAttrs->borderStyle.flagDefined) {
		if (!flagOnLayout) {
			view->resetStateBorders(sl_false);
		}
	}

	{
		ButtonState states[] = {ButtonState::Normal, ButtonState::Hover, ButtonState::Down, ButtonState::Disabled};
		for (sl_uint32 i = 0; i < SLIB_SAPP_LAYOUT_BUTTON_CATEGORY_MAX; i++) {
			SAppLayoutButtonCategory& category = attr->categories[i];
			for (sl_uint32 k = 0; k < (sl_uint32)(ButtonState::Count); k++) {
				if (category.textColor[k].flagDefined) {
					if (!flagOnLayout) {
						view->setTextColor(category.textColor[k].value, states[k], i, sl_false);
					}
					flagRequireNoNative = sl_true;
				}
				if (category.backgroundColor[k].flagDefined) {
					if (!flagOnLayout) {
						view->setBackgroundColor(category.backgroundColor[k].value, states[k], i, sl_false);
					}
					flagRequireNoNative = sl_true;
				}
				if (category.background[k].flagDefined) {
					if (!(_checkDrawableValueAvailable(category.background[k], item->element))) {
						return sl_false;
					}
					if (flagOnLayout) {
						view->setBackground(_getDrawableValue(category.background[k]), states[k], i, sl_false);
					}
					flagRequireNoNative = sl_true;
				}
				if (category.icon[k].flagDefined) {
					if (!(_checkDrawableValueAvailable(category.icon[k], item->element))) {
						return sl_false;
					}
					if (flagOnLayout) {
						view->setIcon(_getDrawableValue(category.icon[k]), states[k], i, sl_false);
					}
					flagRequireNoNative = sl_true;
				}
				if (category.borderWidth[k].flagDefined || category.borderColor[k].flagDefined || category.borderStyle[k].flagDefined) {
					if (category.borderWidth[k].flagDefined && Math::isAlmostZero(category.borderWidth[k].amount)) {
						if (!flagOnLayout) {
							view->setBorder(Ref<Pen>::null(), states[k], i, sl_false);
						}
					} else {
						if (flagOnLayout) {
							sl_real _borderWidth;
							Color _borderColor;
							PenStyle _borderStyle;
							if (category.borderWidth[k].flagDefined) {
								_borderWidth = _getDimensionValue(category.borderWidth[k]);
							} else {
								_borderWidth = 1;
							}
							if (category.borderColor[k].flagDefined) {
								_borderColor = category.borderColor[k].value;
							} else {
								_borderColor = Color::Black;
							}
							if (category.borderStyle[k].flagDefined) {
								_borderStyle = category.borderStyle[k].value;
							} else {
								_borderStyle = PenStyle::Solid;
							}
							view->setBorder(Pen::create(_borderStyle, _borderWidth, _borderColor), states[k], i, sl_false);
						}
					}
					flagRequireNoNative = sl_true;
				}
			}
		}
	}
	
	if (flagRequireNoNative) {
		if (!(item->viewAttrs->nativeWidget.flagDefined)) {
			if (!flagOnLayout) {
				view->setCreatingNativeWidget(sl_false);
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceLabelAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutLabelAttributes> attr = new SAppLayoutLabelAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->labelAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, text)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textColor)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, gravity)

	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppLabelView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutLabelAttributes* attr = item->labelAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	if (!(_checkStringValueAvailable(attr->text, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, text, setText, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, sbDefineInit)
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetLabelAttributes(SAppLayoutSimulationWindow* simulator, LabelView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutLabelAttributes* attr = item->labelAttrs.ptr;
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	if (!(_checkStringValueAvailable(attr->text, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, text, setText, _getString)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, _get)

	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceCheckAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutCheckAttributes> attr = new SAppLayoutCheckAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->checkAttrs = attr;
	
	if (!(_parseLayoutResourceButtonAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, checked)

	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppCheckBox(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutCheckAttributes* attr = item->checkAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppButton(name, item, sbDeclare, sbDefineInit, sbDefineLayout, String::null()))) {
		return sl_false;
	}
	
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, checked, setChecked, sbDefineInit)
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetCheckAttributes(SAppLayoutSimulationWindow* simulator, CheckBox* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutCheckAttributes* attr = item->checkAttrs.ptr;
	
	if (!(_simulateLayoutSetButtonAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, checked, setChecked, _get)
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceRadioAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutRadioAttributes> attr = new SAppLayoutRadioAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->radioAttrs = attr;
	
	if (!(_parseLayoutResourceCheckAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	attr->group = item->getXmlAttribute("group");
	if (attr->group.isNotEmpty()) {
		if (!(SDevUtil::checkName(attr->group.getData(), attr->group.getLength()))) {
			_logError(element, _g_sdev_sapp_error_resource_layout_name_invalid.arg(attr->group));
			return sl_false;
		}
		layout->radioGroups.put(attr->group, sl_true);
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppRadioButton(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutRadioAttributes* attr = item->radioAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppCheckBox(name, item, sbDeclare, sbDefineInit, sbDefineLayout, String::null()))) {
		return sl_false;
	}
	
	if (attr->group.isNotEmpty()) {
		sbDefineInit.add(String::format("%s" RADIOGROUP_NAME_PREFIX "%s->add(%s);%n", strTab, attr->group, name));
	}
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetRadioAttributes(SAppLayoutSimulationWindow* simulator, RadioButton* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutRadioAttributes* attr = item->radioAttrs.ptr;
	
	if (!(_simulateLayoutSetCheckAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	if (!flagOnLayout) {
		if (attr->group.isNotEmpty()) {
			Ref<RadioGroup> group = simulator->getRadioGroup(attr->group);
			if (group.isNotNull()) {
				group->add(view);
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceEditAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutEditAttributes> attr = new SAppLayoutEditAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->editAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, text)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, gravity)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, hintText)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, readOnly)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, multiLine)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textColor)
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppEditView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutEditAttributes* attr = item->editAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	if (!(_checkStringValueAvailable(attr->text, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, text, setText, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, sbDefineInit)
	if (!(_checkStringValueAvailable(attr->hintText, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, hintText, setHintText, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, readOnly, setReadOnly, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, multiLine, setMultiLine, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, sbDefineInit)
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetEditAttributes(SAppLayoutSimulationWindow* simulator, EditView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutEditAttributes* attr = item->editAttrs.ptr;
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	if (!(_checkStringValueAvailable(attr->text, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, text, setText, _getString)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, _get)
	if (!(_checkStringValueAvailable(attr->hintText, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, hintText, setHintText, _getString)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, readOnly, setReadOnly, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, multiLine, setMultiLine, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, _get)
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceImageAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutImageAttributes> attr = new SAppLayoutImageAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->imageAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, src)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, gravity)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scale)
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppImageView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutImageAttributes* attr = item->imageAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	if (!(_checkDrawableValueAvailable(attr->src, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, src, setSource, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, sbDefineInit)
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, scale, setScaleMode, sbDefineInit)
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetImageAttributes(SAppLayoutSimulationWindow* simulator, ImageView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutImageAttributes* attr = item->imageAttrs.ptr;
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	if (!(_checkDrawableValueAvailable(attr->src, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_DRAWABLE(attr->, src, setSource)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, gravity, setGravity, _get)
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, scale, setScaleMode, _get)
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceSelectAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutSelectAttributes> attr = new SAppLayoutSelectAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->selectAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconWidth)
	if (!(attr->iconWidth.checkScalarSize())) {
		LOG_ERROR_LAYOUT_ATTR(iconWidth)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, iconHeight)
	if (!(attr->iconHeight.checkScalarSize())) {
		LOG_ERROR_LAYOUT_ATTR(iconHeight)
		return sl_false;
	}
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, leftIcon)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, rightIcon)
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, textColor)
	
	ListItems< Ref<XmlElement> > itemXmls = item->getChildElements("item");
	for (sl_size i = 0; i < itemXmls.count; i++) {
		Ref<XmlElement>& itemXml = itemXmls[i];
		if (itemXml.isNotNull()) {
			SAppLayoutSelectItem item;
			PARSE_AND_CHECK_LAYOUT_XML_ATTR(itemXml, item., title)
			PARSE_AND_CHECK_LAYOUT_XML_ATTR(itemXml, item., value)
			PARSE_AND_CHECK_LAYOUT_XML_ATTR(itemXml, item., selected)
			
			if (!(attr->items.add(item))) {
				_logError(itemXml, _g_sdev_sapp_error_out_of_memory);
				return sl_false;
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppSelectView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutSelectAttributes* attr = item->selectAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	sl_bool flagRequireNoNative = sl_false;
	
	if (!(_checkDrawableValueAvailable(attr->leftIcon, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, leftIcon, setLeftIcon, sbDefineInit)
	if (attr->leftIcon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (!(_checkDrawableValueAvailable(attr->rightIcon, item->element))) {
		return sl_false;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, rightIcon, setRightIcon, sbDefineInit)
	if (attr->rightIcon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconWidth, setIconWidth, sbDefineInit, sbDefineLayout)
	if (attr->iconWidth.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_LAYOUT(attr->, iconHeight, setIconHeight, sbDefineInit, sbDefineLayout)
	if (attr->iconHeight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, sbDefineInit)
	if (attr->textColor.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (flagRequireNoNative) {
		if (!(item->viewAttrs->nativeWidget.flagDefined)) {
			sbDefineInit.add(String::format("%s%s->setCreatingNativeWidget(sl_false);%n", strTab, name));
		}
	}
	
	ListLocker<SAppLayoutSelectItem> selectItems(attr->items);
	if (selectItems.count > 0) {
		sl_size indexSelected = 0;
		sl_bool flagSelected = sl_false;
		sbDefineInit.add(String::format("%s%s->setItemsCount(%d, sl_false);%n", strTab, name, selectItems.count));
		for (sl_size i = 0; i < selectItems.count; i++) {
			SAppLayoutSelectItem& selectItem = selectItems[i];
			if (!(_checkStringValueAvailable(selectItem.title, item->element))) {
				return sl_false;
			}
			if (selectItem.title.flagDefined) {
				sbDefineInit.add(String::format("%s%s->setItemTitle(%d, %s, sl_false);%n", strTab, name, i, selectItem.title.getAccessString()));
			}
			if (!(_checkStringValueAvailable(selectItem.value, item->element))) {
				return sl_false;
			}
			if (selectItem.value.flagDefined) {
				sbDefineInit.add(String::format("%s%s->setItemValue(%d, %s);%n", strTab, name, i, selectItem.value.getAccessString()));
			}
			if (selectItem.selected.flagDefined && selectItem.selected.value) {
				flagSelected = sl_true;
				indexSelected = i;
			}
		}
		if (flagSelected) {
			sbDefineInit.add(String::format("%s%s->selectIndex(%d, sl_false);%n", strTab, name, indexSelected));
		}
	}
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetSelectAttributes(SAppLayoutSimulationWindow* simulator, SelectView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutSelectAttributes* attr = item->selectAttrs.ptr;
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	sl_bool flagRequireNoNative = sl_false;
	
	if (!(_checkDrawableValueAvailable(attr->leftIcon, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_DRAWABLE(attr->, leftIcon, setLeftIcon)
	if (attr->leftIcon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (!(_checkDrawableValueAvailable(attr->rightIcon, item->element))) {
		return sl_false;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_DRAWABLE(attr->, rightIcon, setRightIcon)
	if (attr->rightIcon.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconWidth, setIconWidth)
	if (attr->iconWidth.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_ONLAYOUT(attr->, iconHeight, setIconHeight)
	if (attr->iconHeight.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, textColor, setTextColor, _get)
	if (attr->textColor.flagDefined) {
		flagRequireNoNative = sl_true;
	}
	
	if (flagRequireNoNative) {
		if (!(item->viewAttrs->nativeWidget.flagDefined)) {
			if (!flagOnLayout) {
				view->setCreatingNativeWidget(sl_false);
			}
		}
	}
	
	if (!flagOnLayout) {
		ListLocker<SAppLayoutSelectItem> selectItems(attr->items);
		if (selectItems.count > 0) {
			sl_uint32 indexSelected = 0;
			sl_bool flagSelected = sl_false;
			sl_uint32 n = (sl_uint32)(selectItems.count);
			view->setItemsCount(n, sl_false);
			for (sl_uint32 i = 0; i < n; i++) {
				SAppLayoutSelectItem& selectItem = selectItems[i];
				if (!(_checkStringValueAvailable(selectItem.title, item->element))) {
					return sl_false;
				}
				if (selectItem.title.flagDefined) {
					view->setItemTitle(i, _getStringValue(selectItem.title), sl_false);
				}
				if (!(_checkStringValueAvailable(selectItem.value, item->element))) {
					return sl_false;
				}
				if (selectItem.value.flagDefined) {
					view->setItemValue(i, _getStringValue(selectItem.value));
				}
				if (selectItem.selected.flagDefined && selectItem.selected.value) {
					flagSelected = sl_true;
					indexSelected = i;
				}
			}
			if (flagSelected) {
				view->selectIndex(indexSelected, sl_false);
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceScrollAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutScrollAttributes> attr = new SAppLayoutScrollAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, scrolling)
	
	item->scrollAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	sl_size nChildren = element->getChildElementsCount();
	if (nChildren > 0) {
		if (nChildren != 1) {
			_logError(element, _g_sdev_sapp_error_resource_layout_scrollview_must_contain_one_child);
			return sl_false;
		}
		Ref<XmlElement> xmlChild = element->getFirstChildElement();
		if (xmlChild.isNotNull()) {
			Ref<SAppLayoutResourceItem> contentItem = _parseLayoutResourceItemChild(layout, item, xmlChild);
			if (contentItem.isNull()) {
				return sl_false;
			}
			attr->content = contentItem;
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppScrollView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutScrollAttributes* attr = item->scrollAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (attr->scrolling.horizontal || attr->scrolling.vertical) {
		sbDefineInit.add(String::format("%s%s->setHorizontalScrolling(%s);%n", strTab, name, (attr->scrolling.horizontal?"sl_true":"sl_false")));
		sbDefineInit.add(String::format("%s%s->setVerticalScrolling(%s);%n", strTab, name, (attr->scrolling.vertical?"sl_true":"sl_false")));
	}
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	sbDefineInit.add(addStatement);
	
	if (attr->content.isNotNull()) {
		String addChildStatement = String::format("%s%s->setContentView(%s, sl_false);%n%n", strTab, name, attr->content->name);
		if (!(_generateLayoutsCppItem(item, attr->content.ptr, sbDeclare, sbDefineInit, sbDefineLayout, addChildStatement))) {
			return sl_false;
		}
	}

	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetScrollAttributes(SAppLayoutSimulationWindow* simulator, ScrollView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutScrollAttributes* attr = item->scrollAttrs.ptr;
	
	if (!flagOnLayout) {
		if (attr->scrolling.horizontal || attr->scrolling.vertical) {
			view->setHorizontalScrolling(attr->scrolling.horizontal?sl_true:sl_false);
			view->setVerticalScrolling(attr->scrolling.vertical?sl_true:sl_false);
		}
	}
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	if (attr->content.isNotNull()) {
		Ref<View> contentView = _simulateLayoutCreateOrLayoutView(simulator, attr->content.ptr, view, flagOnLayout);
		if (!flagOnLayout) {
			if (contentView.isNotNull()) {
				view->setContentView(contentView, sl_false);
			} else {
				return sl_false;
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceLinearAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutLinearAttributes> attr = new SAppLayoutLinearAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	PARSE_AND_CHECK_LAYOUT_ATTR(attr->, orientation)
	
	item->linearAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}

	if (!(_parseLayoutResourceItemChildren(layout, item))) {
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppLinearView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutLinearAttributes* attr = item->linearAttrs.ptr;
	String strTab = "\t\t\t";
	
	GENERATE_CPP_SET_LAYOUT_ATTR_NOREDRAW(attr->, orientation, setOrientation, sbDefineInit)
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetLinearAttributes(SAppLayoutSimulationWindow* simulator, LinearView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutLinearAttributes* attr = item->linearAttrs.ptr;
	
	SIMULATE_LAYOUT_SET_LAYOUT_ATTR_NOREDRAW(attr->, orientation, setOrientation, _get)
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceListAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutListAttributes> attr = new SAppLayoutListAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->listAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppListView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	//SAppLayoutListAttributes* attr = item->listAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetListAttributes(SAppLayoutSimulationWindow* simulator, ListView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	//SAppLayoutListAttributes* attr = item->listAttrs.ptr;
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	return sl_true;
}

sl_bool SAppDocument::_parseLayoutResourceListReportAttributes(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent)
{
	Ref<XmlElement> element = item->element;
	if (element.isNull()) {
		return sl_false;
	}
	
	Ref<SAppLayoutListReportAttributes> attr = new SAppLayoutListReportAttributes;
	if (attr.isNull()) {
		_logError(element, _g_sdev_sapp_error_out_of_memory);
		return sl_false;
	}
	
	item->listReportAttrs = attr;
	
	if (!(_parseLayoutResourceViewAttributes(layout, item, parent))) {
		return sl_false;
	}
	
	ListLocker< Ref<XmlElement> > columnXmls = item->getChildElements("column");
	for (sl_size i = 0; i < columnXmls.count; i++) {
		Ref<XmlElement>& columnXml = columnXmls[i];
		if (columnXml.isNotNull()) {
			SAppLayoutListReportColumn column;
			PARSE_AND_CHECK_LAYOUT_XML_ATTR(columnXml, column., title)
			PARSE_AND_CHECK_LAYOUT_XML_ATTR(columnXml, column., width)
			if (!(column.width.checkScalarSize())) {
				LOG_ERROR_LAYOUT_XML_ATTR(columnXml, width)
				return sl_false;
			}
			PARSE_AND_CHECK_LAYOUT_XML_ATTR(columnXml, column., align)
			PARSE_AND_CHECK_LAYOUT_XML_ATTR(columnXml, column., headerAlign)
			
			if (!(attr->columns.add(column))) {
				_logError(columnXml, _g_sdev_sapp_error_out_of_memory);
				return sl_false;
			}
		}
	}
	
	return sl_true;
}

sl_bool SAppDocument::_generateLayoutsCppListReportView(const String& name, SAppLayoutResourceItem* item, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
{
	SAppLayoutListReportAttributes* attr = item->listReportAttrs.ptr;
	String strTab = "\t\t\t";
	
	if (!(_generateLayoutsCppViewAttributes(name, item, sbDefineInit, sbDefineLayout))) {
		return sl_false;
	}
	
	ListLocker<SAppLayoutListReportColumn> columns(attr->columns);
	if (columns.count > 0) {
		sbDefineInit.add(String::format("%s%s->setColumnsCount(%d, sl_false);%n", strTab, name, columns.count));
		for (sl_size i = 0; i < columns.count; i++) {
			SAppLayoutListReportColumn& column = columns[i];
			if (!(_checkStringValueAvailable(column.title, item->element))) {
				return sl_false;
			}
			if (column.title.flagDefined) {
				sbDefineInit.add(String::format("%s%s->setHeaderText(%d, %s, sl_false);%n", strTab, name, i, column.title.getAccessString()));
			}
			if (column.width.flagDefined) {
				if (column.width.isNeededOnLayoutFunction()) {
					sbDefineLayout.add(String::format("%s%s->setColumnWidth(%d, %s, sl_false);%n", strTab, name, i, column.width.getAccessString()));
				} else {
					sbDefineInit.add(String::format("%s%s->setColumnWidth(%d, %s, sl_false);%n", strTab, name, i, column.width.getAccessString()));
				}
			}
			if (column.align.flagDefined) {
				sbDefineInit.add(String::format("%s%s->setColumnAlignment(%d, %s, sl_false);%n", strTab, name, i, column.align.getAccessString()));
			}
			if (column.headerAlign.flagDefined) {
				sbDefineInit.add(String::format("%s%s->setHeaderAlignment(%d, %s, sl_false);%n", strTab, name, i, column.headerAlign.getAccessString()));
			}
		}
	}
	
	sbDefineInit.add(addStatement);
	
	return sl_true;
}

sl_bool SAppDocument::_simulateLayoutSetListReportAttributes(SAppLayoutSimulationWindow* simulator, ListReportView* view, SAppLayoutResourceItem* item, sl_bool flagOnLayout)
{
	SAppLayoutListReportAttributes* attr = item->listReportAttrs.ptr;
	
	if (!(_simulateLayoutSetViewAttributes(simulator, view, item, flagOnLayout))) {
		return sl_false;
	}
	
	ListLocker<SAppLayoutListReportColumn> columns(attr->columns);
	if (columns.count > 0) {
		sl_uint32 n = (sl_uint32)(columns.count);
		if (!flagOnLayout) {
			view->setColumnsCount(n, sl_false);
		}
		for (sl_uint32 i = 0; i < n; i++) {
			SAppLayoutListReportColumn& column = columns[i];
			if (!flagOnLayout) {
				if (!(_checkStringValueAvailable(column.title, item->element))) {
					return sl_false;
				}
				if (column.title.flagDefined) {
					view->setHeaderText(i, _getStringValue(column.title), sl_false);
				}
				if (column.align.flagDefined) {
					view->setColumnAlignment(i, column.align.value, sl_false);
				}
				if (column.headerAlign.flagDefined) {
					view->setHeaderAlignment(i, column.headerAlign.value, sl_false);
				}
			}
			if (column.width.flagDefined) {
				if (flagOnLayout) {
					view->setColumnWidth(i, _getDimensionValue(column.width), sl_false);
				}
			}
		}
	}
	
	return sl_true;
}

SLIB_SDEV_NAMESPACE_END
