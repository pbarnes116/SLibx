#include "../../../inc/slibx/sdev/sapp.h"

#include "../../../inc/slibx/sdev/util.h"

#include <slib/core/file.h>
#include <slib/core/log.h>
#include <slib/core/resource.h>
#include <slib/core/string_buffer.h>
#include <slib/core/scoped.h>
#include <slib/ui.h>

#define TAG "SApp"

namespace slib
{

	SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_begin, "Opening app configuration file: %s")
	SLIB_STATIC_STRING(_g_sdev_sapp_log_appconf_end, "Finished app configuration")
	SLIB_STATIC_STRING(_g_sdev_sapp_log_open_drawables_begin, "Opening drwable resources: %s")
	SLIB_STATIC_STRING(_g_sdev_sapp_log_open_raws_begin, "Opening raw resources: %s")
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
	SLIB_STATIC_STRING(_g_sdev_sapp_error_layout_style_not_found, "layout-style is not found: %s")
	SLIB_STATIC_STRING(_g_sdev_sapp_error_layout_include_not_found, "layout-include is not found: %s")
	SLIB_STATIC_STRING(_g_sdev_sapp_error_layout_not_found, "layout is not found: %s")
	SLIB_STATIC_STRING(_g_sdev_sapp_error_layout_is_not_view, "layout is not a view: %s")

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
	SLIB_STATIC_STRING(_g_sdev_sapp_error_resource_layout_item_must_contain_one_child, "Layout Resource: item can contain only one child")
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
			for (auto fileName : File::getFiles(m_pathApp)) {
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
					} else if (fileName == "raw") {
						if (!(_registerRawResources("raw", m_pathApp + "/raw"))) {
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
				for (auto fileName : File::getFiles(pathDir)) {
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
		
		return m_layouts.getAllValues();
		
	}

	void SAppDocument::simulateLayoutInWindow(const String& layoutName)
	{
		ObjectLocker lock(this);
		
		if (!m_flagOpened) {
			return;
		}

		Ref<SAppLayoutResource> layout = m_layouts.getValue(layoutName, Ref<SAppLayoutResource>::null());
		if (layout.isNotNull()) {
			_simulateLayoutInWindow(layout.get());
		}
		
	}

	/***************************************************
						Log
	***************************************************/

	void SAppDocument::_log(const String& text)
	{
		Log(TAG, text);
	}

	void SAppDocument::_logError(const String& text)
	{
		LogError(TAG, text);
	}

	void SAppDocument::_logError(const String& filePath, sl_size line, sl_size col, const String& text)
	{
		LogError(TAG, "%s(%d:%d)%n%s", filePath, line, col, text);
	}

	void SAppDocument::_logError(const Ref<XmlElement>& element, const String& text)
	{
		if (element.isNotNull()) {
			LogError(TAG, "%s(%d:%d)%n%s", element->getSourceFilePath(), element->getLineNumberInSource(), element->getColumnNumberInSource(), text);
		} else {
			LogError(TAG, text);
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
		param.flagLogError = sl_false;
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
					if (child.isNotNull()) {
						if (child->getName() == "include-header") {
							String str = child->getText().trim();
							if (str.isNotEmpty()) {
								conf.generate_cpp_layout_include_headers.add(str);
							}
						} else if (child->getName() == "include-header-in-cpp") {
							String str = child->getText().trim();
							if (str.isNotEmpty()) {
								conf.generate_cpp_layout_include_headers_in_cpp.add(str);
							}
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
		m_layoutIncludes.removeAll();
		
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
		param.flagLogError = sl_false;
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
				} else if (child->getName() == "layout-include") {
					if (!_parseLayoutInclude(child)) {
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

	sl_bool SAppDocument::_registerRawResources(const String& resourcePath, const String& fileDirPath)
	{
		_log(_g_sdev_sapp_log_open_raws_begin.arg(fileDirPath));
		List<String> _list = File::getFiles(fileDirPath);
		_list.sort();
		ListElements<String> list(_list);
		for (sl_size i = 0; i < list.count; i++) {
			const String& fileName = list[i];
			if (fileName.isNotEmpty() && !(fileName.startsWith('.'))) {
				String name;
				if (!(_registerRawResource(fileName, fileDirPath + "/" + fileName, name))) {
					return sl_false;
				}
			}
		}
		return sl_true;
	}

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
		
		for (auto pair : m_raws) {
			
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
			for (auto pair : m_drawables) {
				if (pair.value.isNotNull()) {
					Ref<SAppDrawableResource>& res = pair.value;
					if (res->type == SAppDrawableResource::typeImage) {
						_generateDrawablesCpp_Image(res.get(), sbHeader, sbCpp, sbMap);
					}
				}
			}
		}
		
		// iterate other resources
		{
			for (auto pair : m_drawables) {
				if (pair.value.isNotNull()) {
					Ref<SAppDrawableResource>& res = pair.value;
					if (res->type == SAppDrawableResource::typeNinePieces) {
						_generateDrawablesCpp_NinePieces(res.get(), sbHeader, sbCpp, sbMap);
					} else if (res->type == SAppDrawableResource::typeNinePatch) {
						_generateDrawablesCpp_NinePatch(res.get(), sbHeader, sbCpp, sbMap);
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
		if (value.flagColor) {
			return Drawable::createColorDrawable(value.color);
		}
		Ref<SAppDrawableResource> res = m_drawables.getValue(value.resourceName, Ref<SAppDrawableResource>::null());
		if (res.isNull()) {
			_logError(_g_sdev_sapp_error_drawable_not_found.arg(value.resourceName));
			return Ref<Drawable>::null();
		}
		
		Ref<Drawable> drawable;
		if (res->type == SAppDrawableResource::typeImage) {
			drawable = _getDrawableValue_Image(res.get());
		} else if (res->type == SAppDrawableResource::typeNinePieces) {
			drawable = _getDrawableValue_NinePieces(res.get());
		} else if (res->type == SAppDrawableResource::typeNinePatch) {
			drawable = _getDrawableValue_NinePatch(res.get());
		}
		if (drawable.isNotNull()) {
			if (!(value.flagWhole)) {
				drawable = Drawable::createSubDrawable(drawable, value.x, value.y, value.width, value.height);
			}
			if (value.func == SAppDrawableValue::FUNC_NINEPATCH) {
				drawable = NinePatchDrawable::create(_getDimensionIntValue(value.patchLeftWidthDst), _getDimensionIntValue(value.patchRightWidthDst), _getDimensionIntValue(value.patchTopHeightDst), _getDimensionIntValue(value.patchBottomHeightDst), drawable, value.patchLeftWidth, value.patchRightWidth, value.patchTopHeight, value.patchBottomHeight);
			} else if (value.func == SAppDrawableValue::FUNC_THREEPATCH_HORIZONTAL) {
				drawable = HorizontalThreePatchDrawable::create(_getDimensionIntValue(value.patchLeftWidthDst), _getDimensionIntValue(value.patchRightWidthDst), drawable, value.patchLeftWidth, value.patchRightWidth);
			} else if (value.func == SAppDrawableValue::FUNC_THREEPATCH_VERTICAL) {
				drawable = VerticalThreePatchDrawable::create(_getDimensionIntValue(value.patchTopHeightDst), _getDimensionIntValue(value.patchBottomHeightDst), drawable, value.patchTopHeight, value.patchBottomHeight);
			}
		}
		return drawable;
		
	}

	Ref<Image> SAppDocument::_getImageValue(SAppDrawableValue& value)
	{
		if (!(value.flagDefined) || value.flagNull || value.flagColor || !(value.flagWhole) || value.func != SAppDrawableValue::FUNC_NONE) {
			return Ref<Image>::null();
		}
		Ref<Drawable> drawable = _getDrawableValue(value);
		if (IsInstanceOf<Image>(drawable)) {
			return Ref<Image>::from(drawable);
		} else if (MipmapDrawable* mipmap = CastInstance<MipmapDrawable>(drawable.get())) {
			drawable = mipmap->getSource(0);
			return CastRef<Image>(drawable);
		}
		return Ref<Image>::null();
	}

	sl_bool SAppDocument::_registerImageResources(const String& resourcePath, const String& fileDirPath, const Locale& locale)
	{
		_log(_g_sdev_sapp_log_open_drawables_begin.arg(fileDirPath));
		List<String> _list = File::getFiles(fileDirPath);
		_list.sort();
		ListElements<String> list(_list);
		for (sl_size i = 0; i < list.count; i++) {
			const String& fileName = list[i];
			if (!(fileName.startsWith('.'))) {
				String ext = File::getFileExtension(fileName);
				if (ext == "png" || ext == "jpg" || ext == "jpeg") {
					String name = File::getFileNameOnly(fileName);
					sl_reg indexSharp = name.indexOf('#');
					sl_bool flagMain = sl_true;
					if (indexSharp >= 0) {
						flagMain = sl_false;
						name = name.substring(0, indexSharp);
					}
					if (name.isEmpty()) {
						_logError(_g_sdev_sapp_error_resource_drawable_filename_invalid.arg(resourcePath + "/" + fileName));
						return sl_false;
					}
					name = Resources::makeResourceName(name);
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
					
					SAppDrawableResourceImageAttributes* imageAttr = res->imageAttrs.get();
					
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
		}
		return sl_true;
	}

	sl_bool SAppDocument::_generateDrawablesCpp_Image(SAppDrawableResource* res, StringBuffer& sbHeader, StringBuffer& sbCpp, StringBuffer& sbMap)
	{
		SAppDrawableResourceImageAttributes* imageAttr = res->imageAttrs.get();
		
		Ref<SAppDrawableResourceImageItem> item;
		
		sl_bool flagSuccess = sl_false;
		
		sl_uint32 defaultWidth = 0;
		sl_uint32 defaultHeight = 0;
		
		if (imageAttr->defaultImages.getAt(0, &item)) {
			
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
				for (auto pairItems : imageAttr->images) {
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
				for (auto pairItems : imageAttr->images) {
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
			
			ListElements< Pair<Locale, List< Ref<SAppDrawableResourceImageItem> > > > pairs(listPairs);
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
		SAppDrawableResourceImageAttributes* imageAttr = res->imageAttrs.get();
		
		Ref<SAppDrawableResourceImageItem> item;

		sl_size n = imageAttr->defaultImages.getCount();
		if (n == 1) {
			if (imageAttr->defaultImages.getAt(0, &item)) {
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
		
		SAppDrawableResourceNinePiecesAttributes* attr = res->ninePiecesAttrs.get();

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
		SAppDrawableResourceNinePiecesAttributes* attr = res->ninePiecesAttrs.get();
		
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
		SAppDrawableResourceNinePiecesAttributes* attr = res->ninePiecesAttrs.get();
		
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
			
			return NinePiecesDrawable::create(_getDimensionIntValue(attr->leftWidth), _getDimensionIntValue(attr->rightWidth), _getDimensionIntValue(attr->topHeight), _getDimensionIntValue(attr->bottomHeight), _getDrawableValue(attr->topLeft), _getDrawableValue(attr->top), _getDrawableValue(attr->topRight), _getDrawableValue(attr->left), _getDrawableValue(attr->center), _getDrawableValue(attr->right), _getDrawableValue(attr->bottomLeft), _getDrawableValue(attr->bottom), _getDrawableValue(attr->bottomRight));
			
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
		
		SAppDrawableResourceNinePatchAttributes* attr = res->ninePatchAttrs.get();
		
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
		SAppDrawableResourceNinePatchAttributes* attr = res->ninePatchAttrs.get();
		
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
		SAppDrawableResourceNinePatchAttributes* attr = res->ninePatchAttrs.get();
		if (!(_checkDrawableValueAvailable(attr->src, attr->element))) {
			return Ref<Drawable>::null();
		}
		return NinePatchDrawable::create(_getDimensionIntValue(attr->dstLeftWidth), _getDimensionIntValue(attr->dstRightWidth), _getDimensionIntValue(attr->dstTopHeight), _getDimensionIntValue(attr->dstBottomHeight), _getDrawableValue(attr->src), attr->leftWidth, attr->rightWidth, attr->topHeight, attr->bottomHeight);
	}

	sl_bool SAppDocument::_checkDrawableValueAvailable(SAppDrawableValue& value, const Ref<XmlElement>& elementError)
	{
		if (!(value.flagDefined)) {
			return sl_true;
		}
		if (value.flagNull) {
			return sl_true;
		}
		if (value.flagColor) {
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
		if (value.flagColor || !(value.flagWhole) || value.func != SAppDrawableValue::FUNC_NONE) {
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
		
		for (auto pair : m_strings) {
			
			if (pair.value.isNotNull()) {
				
				sbHeader.add(String::format("\t\tSLIB_DECLARE_STRING_RESOURCE(%s)%n", pair.key));
				
				String defValue = pair.value->defaultValue;
				
				if (pair.value->values.isEmpty()) {
					sbCpp.add(String::format("\t\tSLIB_DEFINE_STRING_RESOURCE_SIMPLE(%s, \"%s\")%n%n", pair.key, ParseUtil::applyBackslashEscapes(defValue, sl_true, sl_false, sl_true)));
				} else {
					
					sbCpp.add(String::format("\t\tSLIB_DEFINE_STRING_RESOURCE_BEGIN(%s, \"%s\")%n", pair.key, ParseUtil::applyBackslashEscapes(defValue, sl_true, sl_false, sl_true)));
					
					// locales
					{
						for (auto pairValues : pair.value->values) {
							if (pairValues.key.getCountry() != Country::Unknown) {
								sbCpp.add(String::format("\t\t\tSLIB_DEFINE_STRING_RESOURCE_VALUE(%s, \"%s\")%n", pairValues.key.toString(), ParseUtil::applyBackslashEscapes(pairValues.value, sl_true, sl_false, sl_true)));
							}
						}
					}
					// languages
					{
						for (auto pairValues : pair.value->values) {
							if (pairValues.key.getCountry() == Country::Unknown) {
								sbCpp.add(String::format("\t\t\tSLIB_DEFINE_STRING_RESOURCE_VALUE(%s, \"%s\")%n", pairValues.key.toString(), ParseUtil::applyBackslashEscapes(pairValues.value, sl_true, sl_false, sl_true)));
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
				Ref<SAppMenuResourceItem> menuItem = _parseMenuResourceItem(child, menu.get(), SAppMenuResourceItem::all_platforms);
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
		
		
		for (auto pair : m_menus) {
			if (pair.value.isNotNull()) {
				
				sbHeader.add(String::format("\t\tSLIB_DECLARE_MENU_BEGIN(%s)%n", pair.key));
				sbCpp.add(String::format("\t\tSLIB_DEFINE_MENU_BEGIN(%s)%n", pair.key));
				
				ListLocker< Ref<SAppMenuResourceItem> > items(pair.value->children);
				for (sl_size i = 0; i < items.count; i++) {
					Ref<SAppMenuResourceItem>& item = items[i];
					if (item.isNotNull()) {
						if (!_generateMenusCpp_Item("root", SAppMenuResourceItem::all_platforms, item.get(), sbHeader, sbCpp, 3)) {
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
					_generateMenusCpp_Item(item->name, item->platformFlags, childItem.get(), sbHeader, sbCpp, tabLevel + 1);
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
				if (!(_getMenuValue_Item(menu, item.get()))) {
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
					if (!(_getMenuValue_Item(submenu.get(), childItem.get()))) {
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

	enum LAYOUT_OP
	{
		OP_PARSE,
		OP_GENERATE_CPP,
		OP_SIMULATE
	};

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

	sl_bool SAppDocument::_parseLayoutInclude(const Ref<XmlElement>& element)
	{
		if (element.isNull()) {
			return sl_false;
		}
		
		Ref<SAppLayoutInclude> include = new SAppLayoutInclude;
		if (include.isNull()) {
			_logError(element, _g_sdev_sapp_error_out_of_memory);
			return sl_false;
		}
		
		include->element = element;
		
		String name = element->getAttribute("name").trim();
		if (name.isEmpty()) {
			_logError(element, _g_sdev_sapp_error_resource_layout_name_is_empty);
			return sl_false;
		}
		if (m_layoutIncludes.contains(name)) {
			_logError(element, _g_sdev_sapp_error_resource_layout_name_redefined.arg(name));
			return sl_false;
		}
		include->name = name;
		
		if (!(m_layoutIncludes.put(name, include))) {
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

		SAppLayoutType type;
		String strType = element->getAttribute("type");
		if (strType.isEmpty() || strType == "view") {
			type = SAppLayoutType::View;
		} else if (strType == "window") {
			type = SAppLayoutType::Window;
		} else if (strType == "page") {
			type = SAppLayoutType::Page;
		} else {
			_logError(element, _g_sdev_sapp_error_resource_layout_type_invalid.arg(strType));
			return sl_false;
		}
		layout->layoutType = type;
		
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
			
		if (!(_parseLayoutResourceItem(layout.get(), layout.get(), sl_null))) {
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
				name = layout->getAutoIncreasingName(item->itemType);
				item->flagGeneratedName = sl_true;
			}
			item->name = name;
		}
		
		LayoutControlProcessParams pp;
		pp.op = OP_PARSE;
		pp.resource = layout;
		pp.resourceItem = item;
		pp.parentResourceItem = parent;
		if (!(_processLayoutResourceControl(&pp))) {
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
		
		if (!parent) {
			String strSP = layout->getXmlAttribute("sp");
			if (!(layout->sp.parse(strSP))) {
				_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("sp", strSP));
			}
			if (!(layout->sp.checkSP())) {
				_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("sp", strSP));
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

	Ref<SAppLayoutResourceItem> SAppDocument::_parseLayoutResourceItemChild(SAppLayoutResource* layout, SAppLayoutResourceItem* parentItem, const Ref<XmlElement>& element)
	{
		String strType = element->getName();
		SAppLayoutItemType type = SAppLayoutResource::getTypeFromName(strType);
		if (type == SAppLayoutItemType::Unknown) {
			_logError(element, _g_sdev_sapp_error_resource_layout_type_invalid.arg(strType));
			return Ref<SAppLayoutResourceItem>::null();
		}
		
		Ref<SAppLayoutResourceItem> childItem = new SAppLayoutResourceItem;
		if (childItem.isNull()) {
			_logError(element, _g_sdev_sapp_error_out_of_memory);
			return Ref<SAppLayoutResourceItem>::null();
		}
		
		childItem->itemType = type;
		childItem->element = element;
		
		if (!(_parseLayoutResourceItem(layout, childItem.get(), parentItem))) {
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
									"namespace %s%n"
									"{%n\tnamespace ui%n\t{%n"
									, m_conf.generate_cpp_namespace));
		
		sbCpp.add(String::format(
								"#include <slib/ui.h>%n"
								 "#include \"layouts.h\"%n"
								 "#include \"strings.h\"%n"
								 "#include \"drawables.h\"%n"
								 "#include \"menus.h\"%n"
								 , m_conf.generate_cpp_namespace));
		
		{
			ListLocker<String> includes(m_conf.generate_cpp_layout_include_headers_in_cpp);
			for (sl_size i = 0; i < includes.count; i++) {
				if (includes[i].isNotEmpty()) {
					sbCpp.add(String::format("#include \"%s\"%n", includes[i]));
				}
			}
		}
		sbCpp.add(String::format(
									"%n"
									"namespace %s%n"
									"{%n\tnamespace ui%n\t{%n"
									, m_conf.generate_cpp_namespace));

		{
			for (auto pair : m_layouts) {
				if (pair.value.isNotNull()) {
					Ref<SAppLayoutResource> layout = pair.value;
					sbHeader.add(String::format("\t\tclass %s;%n", pair.key));
				}
			}
		}
		
		sbHeader.add("\r\n");
		
		{
			for (auto pair : m_layouts) {
				
				if (pair.value.isNotNull()) {
					
					Ref<SAppLayoutResource> layout = pair.value;
					
					if (layout->layoutType == SAppLayoutType::Window) {
						sbHeader.add(String::format("\t\tSLIB_DECLARE_WINDOW_LAYOUT_BEGIN(%s)%n", pair.key));
						sbCpp.add(String::format("\t\tSLIB_DEFINE_WINDOW_LAYOUT(%s)%n%n", pair.key));
					} else if (layout->layoutType == SAppLayoutType::Page) {
						sbHeader.add(String::format("\t\tSLIB_DECLARE_PAGE_LAYOUT_BEGIN(%s)%n", pair.key));
						sbCpp.add(String::format("\t\tSLIB_DEFINE_PAGE_LAYOUT(%s)%n%n", pair.key));
					} else if (layout->layoutType == SAppLayoutType::View) {
						sbHeader.add(String::format("\t\tSLIB_DECLARE_VIEW_LAYOUT_BEGIN(%s)%n", pair.key));
						sbCpp.add(String::format("\t\tSLIB_DEFINE_VIEW_LAYOUT(%s)%n%n", pair.key));
					} else {
						return sl_false;
					}
					
					sbCpp.add(String::format("\t\tvoid %s::initialize()%n\t\t{%n", pair.key));
					
					{
						ListElements<String> radioGroups(layout->radioGroups.getAllKeys());
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
					
					if (layout->sp.flagDefined) {
						if (layout->sp.isNeededOnLayoutFunction()) {
							sbLayout.add(String::format("%n\t\t\tsetScaledPixel(%s);%n", layout->sp.getAccessString()));
						} else {
							sbCpp.add(String::format("%n\t\t\tsetScaledPixel(%s);%n%n", layout->sp.getAccessString()));
						}
					}
					
					if (!(_generateLayoutsCppItem(layout.get(), layout.get(), sl_null, sbHeader, sbCpp, sbLayout, String::null()))) {
						return sl_false;
					}
					
					sbCpp.add(String::format("\t\t}%n%n\t\tvoid %s::layoutViews(sl_ui_len CONTENT_WIDTH, sl_ui_len CONTENT_HEIGHT)%n\t\t{%n", pair.key));
					sbCpp.link(sbLayout);
					static sl_char8 strEndCpp[] = "\t\t}\r\n\r\n";
					sbCpp.addStatic(strEndCpp, sizeof(strEndCpp)-1);
					
					if (layout->layoutType == SAppLayoutType::Window) {
						static sl_char8 strEndHeader[] = "\t\tSLIB_DECLARE_WINDOW_LAYOUT_END\r\n\r\n";
						sbHeader.addStatic(strEndHeader, sizeof(strEndHeader)-1);
					} else if (layout->layoutType == SAppLayoutType::Page) {
						static sl_char8 strEndHeader[] = "\t\tSLIB_DECLARE_PAGE_LAYOUT_END\r\n\r\n";
						sbHeader.addStatic(strEndHeader, sizeof(strEndHeader)-1);
					} else if (layout->layoutType == SAppLayoutType::View) {
						static sl_char8 strEndHeader[] = "\t\tSLIB_DECLARE_VIEW_LAYOUT_END\r\n\r\n";
						sbHeader.addStatic(strEndHeader, sizeof(strEndHeader)-1);
					}
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

	sl_bool SAppDocument::_generateLayoutsCppItem(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement)
	{
		String name;
		if (parent) {
			name = item->name;
			sbDeclare.add(String::format("\t\t\tslib::Ref<%s> %s;%n", item->className, name));
			sbDefineInit.add(String::format("\t\t\t%2$s = new %1$s;%n", item->className, name));
		} else {
			name = "this";
		}
		
		LayoutControlProcessParams pp;
		pp.op = OP_GENERATE_CPP;
		pp.resource = layout;
		pp.resourceItem = item;
		pp.parentResourceItem = parent;
		pp.addStatement = addStatement;
		pp.name = name;
		pp.sbDeclare = &sbDeclare;
		pp.sbDefineInit = &sbDefineInit;
		pp.sbDefineLayout = &sbDefineLayout;
		
		if (!(_processLayoutResourceControl(&pp))) {
			return sl_false;
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

	Ref<View> SAppDocument::_simulateLayoutCreateOrLayoutView(SAppLayoutSimulator* simulator, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent, View* parentView, sl_bool flagOnLayout)
	{
		Ref<SAppLayoutSimulationWindow> window = simulator->getSimulationWindow();
		if (window.isNull()) {
			return Ref<View>::null();
		}
		Ref<SAppLayoutResource> layout = simulator->getLayoutResource();
		if (layout.isNull()) {
			return Ref<View>::null();
		}
		
		Ref<View> view;
		if (parent) {
			if (flagOnLayout) {
				view = simulator->getViewByName(item->name);
				if (view.isNull()) {
					return Ref<View>::null();
				}
			}
		} else {
			view = simulator->getSimulationContentView();
			if (view.isNull()) {
				return Ref<View>::null();
			}
			if (layout->layoutType == SAppLayoutType::Window) {
				UISize size = UI::getScreenSize();
				m_layoutSimulationParams.screenWidth = size.x;
				m_layoutSimulationParams.screenHeight = size.y;
				size = window->getClientSize();
				m_layoutSimulationParams.viewportWidth = size.x;
				m_layoutSimulationParams.viewportHeight = size.y;
			} else {
				UISize size = window->getClientSize();
				m_layoutSimulationParams.screenWidth = size.x;
				m_layoutSimulationParams.screenHeight = size.y;
				m_layoutSimulationParams.viewportWidth = view->getWidth();
				m_layoutSimulationParams.viewportHeight = view->getHeight();
			}
			if (layout->sp.flagDefined) {
				m_layoutSimulationParams.sp = _getDimensionFloatValue(layout->sp);
			} else {
				m_layoutSimulationParams.sp = 1;
			}
		}
		
		LayoutControlProcessParams pp;
		pp.op = OP_SIMULATE;
		pp.resource = layout.get();
		pp.resourceItem = item;
		pp.parentResourceItem = parent;
		pp.simulator = simulator;
		pp.window = window.get();
		pp.view = view;
		pp.parentView = parentView;
		pp.flagOnLayout = flagOnLayout;
		if (!(_processLayoutResourceControl(&pp))) {
			return Ref<View>::null();
		}
		
		if (parent) {
			if (!flagOnLayout) {
				view = pp.view;
				if (view.isNull()) {
					return Ref<View>::null();
				}
				simulator->registerViewByName(item->name, view);
			}
		}

		return view;
		
	}

	sl_ui_pos SAppDocument::_getDimensionIntValue(SAppDimensionValue& value)
	{
		if (!(value.flagDefined)) {
			return 0;
		}
		switch (value.unit) {
			case SAppDimensionValue::PX:
				return (sl_ui_pos)(value.amount);
			case SAppDimensionValue::SW:
				return (sl_ui_pos)(value.amount * m_layoutSimulationParams.screenWidth);
			case SAppDimensionValue::SH:
				return (sl_ui_pos)(value.amount * m_layoutSimulationParams.screenHeight);
			case SAppDimensionValue::SMIN:
				return (sl_ui_pos)(value.amount * SLIB_MIN(m_layoutSimulationParams.screenWidth, m_layoutSimulationParams.screenHeight));
			case SAppDimensionValue::SMAX:
				return (sl_ui_pos)(value.amount * SLIB_MAX(m_layoutSimulationParams.screenWidth, m_layoutSimulationParams.screenHeight));
			case SAppDimensionValue::VW:
				return (sl_ui_pos)(value.amount * m_layoutSimulationParams.viewportWidth);
			case SAppDimensionValue::VH:
				return (sl_ui_pos)(value.amount * m_layoutSimulationParams.viewportHeight);
			case SAppDimensionValue::VMIN:
				return (sl_ui_pos)(value.amount * SLIB_MIN(m_layoutSimulationParams.viewportWidth, m_layoutSimulationParams.viewportHeight));
			case SAppDimensionValue::VMAX:
				return (sl_ui_pos)(value.amount * SLIB_MAX(m_layoutSimulationParams.viewportWidth, m_layoutSimulationParams.viewportHeight));
			case SAppDimensionValue::SP:
				return (sl_ui_pos)(value.amount * m_layoutSimulationParams.sp);
		}
		return 0;
	}

	sl_real SAppDocument::_getDimensionFloatValue(SAppDimensionFloatValue& value)
	{
		if (!(value.flagDefined)) {
			return 0;
		}
		switch (value.unit) {
			case SAppDimensionValue::PX:
				return value.amount;
			case SAppDimensionValue::SW:
				return value.amount * (sl_real)(m_layoutSimulationParams.screenWidth);
			case SAppDimensionValue::SH:
				return value.amount * (sl_real)(m_layoutSimulationParams.screenHeight);
			case SAppDimensionValue::SMIN:
				return value.amount * (sl_real)(SLIB_MIN(m_layoutSimulationParams.screenWidth, m_layoutSimulationParams.screenHeight));
			case SAppDimensionValue::SMAX:
				return value.amount * (sl_real)(SLIB_MAX(m_layoutSimulationParams.screenWidth, m_layoutSimulationParams.screenHeight));
			case SAppDimensionValue::VW:
				return value.amount * (sl_real)(m_layoutSimulationParams.viewportWidth);
			case SAppDimensionValue::VH:
				return value.amount * (sl_real)(m_layoutSimulationParams.viewportHeight);
			case SAppDimensionValue::VMIN:
				return value.amount * (sl_real)(SLIB_MIN(m_layoutSimulationParams.viewportWidth, m_layoutSimulationParams.viewportHeight));
			case SAppDimensionValue::VMAX:
				return value.amount * (sl_real)(SLIB_MAX(m_layoutSimulationParams.viewportWidth, m_layoutSimulationParams.viewportHeight));
			case SAppDimensionValue::SP:
				return value.amount * m_layoutSimulationParams.sp;
		}
		return 0;
	}

	List< Ref<XmlElement> > SAppDocument::_getLayoutItemChildElements(SAppLayoutResourceItem* item, const String& tagName)
	{
		List< Ref<XmlElement> > ret;
		{
			ListLocker< Ref<XmlElement> > children(item->element->getChildElements(tagName));
			for (sl_size i = 0; i < children.count; i++) {
				Ref<XmlElement>& child = children[i];
				if (child.isNotNull()) {
					String name = child->getName();
					if (name == "include") {
						String src = child->getAttribute("src");
						if (src.isEmpty()) {
							_logError(child, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("src", name));
							return List< Ref<XmlElement> >::null();
						}
						Ref<SAppLayoutInclude> include;
						m_layoutIncludes.get(src, &include);
						if (include.isNotNull()) {
							ret.addAll(include->element->getChildElements(tagName));
						} else {
							_logError(child, _g_sdev_sapp_error_layout_include_not_found.arg(name));
							return List< Ref<XmlElement> >::null();
						}
					} else {
						ret.add(child);
					}
				}
			}
		}
		{
			ListLocker< Ref<SAppLayoutStyle> > _styles(item->styles);
			for (sl_size i = 0; i < _styles.count; i++) {
				Ref<SAppLayoutStyle> style = _styles[i];
				if (style.isNotNull()) {
					ret.addAll(style->element->getChildElements(tagName));
				}
			}
		}
		return ret;
	}

#define PROCESS_CONTROL_SWITCH(NAME) \
	case SAppLayoutItemType::NAME: \
		if (!(_processLayoutResourceControl_##NAME(params))) { \
			return sl_false; \
		} \
		break;

	sl_bool SAppDocument::_processLayoutResourceControl(LayoutControlProcessParams *params)
	{
		SAppLayoutResourceItem* resourceItem = params->resourceItem;
		int op = params->op;
		switch (resourceItem->itemType)
		{
			case SAppLayoutItemType::ViewGroup:
				{
					if (params->parentResourceItem) {
						if (!(_processLayoutResourceControl_ViewGroup(params))) {
							return sl_false;
						}
					} else {
						if (params->resource->layoutType == SAppLayoutType::Window) {
							if (!(_processLayoutResourceControl_Window(params))) {
								return sl_false;
							}
						} else if (params->resource->layoutType == SAppLayoutType::Page) {
							if (!(_processLayoutResourceControl_Page(params))) {
								return sl_false;
							}
						} else {
							if (!(_processLayoutResourceControl_ViewGroup(params))) {
								return sl_false;
							}
						}
					}
				}
				break;
			PROCESS_CONTROL_SWITCH(View)
			PROCESS_CONTROL_SWITCH(Import)
			PROCESS_CONTROL_SWITCH(Button)
			PROCESS_CONTROL_SWITCH(Label)
			PROCESS_CONTROL_SWITCH(Check)
			PROCESS_CONTROL_SWITCH(Radio)
			PROCESS_CONTROL_SWITCH(Edit)
			PROCESS_CONTROL_SWITCH(Password)
			PROCESS_CONTROL_SWITCH(TextArea)
			PROCESS_CONTROL_SWITCH(Image)
			PROCESS_CONTROL_SWITCH(Select)
			PROCESS_CONTROL_SWITCH(Scroll)
			PROCESS_CONTROL_SWITCH(Linear)
			PROCESS_CONTROL_SWITCH(List)
			PROCESS_CONTROL_SWITCH(ListReport)
			PROCESS_CONTROL_SWITCH(Render)
			PROCESS_CONTROL_SWITCH(Tab)
			PROCESS_CONTROL_SWITCH(Tree)
			PROCESS_CONTROL_SWITCH(Split)
			PROCESS_CONTROL_SWITCH(Web)
			PROCESS_CONTROL_SWITCH(Progress)
			PROCESS_CONTROL_SWITCH(Slider)
			PROCESS_CONTROL_SWITCH(Picker)
			PROCESS_CONTROL_SWITCH(Pager)
			PROCESS_CONTROL_SWITCH(Video)
			default:
				return sl_false;
		}
		
		switch (resourceItem->itemType) {
			case SAppLayoutItemType::ViewGroup:
			case SAppLayoutItemType::Linear:
			case SAppLayoutItemType::Render:
				if (op == OP_PARSE) {
					ListLocker< Ref<XmlElement> > children(_getLayoutItemChildElements(resourceItem, String::null()));
					for (sl_size i = 0; i < children.count; i++) {
						const Ref<XmlElement>& child = children[i];
						if (child.isNotNull()) {
							Ref<SAppLayoutResourceItem> childItem = _parseLayoutResourceItemChild(params->resource, resourceItem, child);
							if (childItem.isNull()) {
								return sl_false;
							}
							if (!(resourceItem->children.add(childItem))) {
								_logError(resourceItem->element, _g_sdev_sapp_error_out_of_memory);
								return sl_false;
							}
						}
					}
					return sl_true;
				} else if (op == OP_GENERATE_CPP) {
					String name;
					if (params->parentResourceItem) {
						name = params->name;
					} else {
						static sl_char8 strEnd[] = "\r\n";
						params->sbDefineInit->addStatic(strEnd, sizeof(strEnd)-1);
						name = "m_contentView";
					}
					ListLocker< Ref<SAppLayoutResourceItem> > children(resourceItem->children);
					for (sl_size i = 0; i < children.count; i++) {
						Ref<SAppLayoutResourceItem>& child = children[i];
						if (child.isNotNull()) {
							String addStatement = String::format("\t\t\t%s->addChild(%s, slib::UIUpdateMode::Init);%n%n", name, child->name);
							if (!(_generateLayoutsCppItem(params->resource, child.get(), resourceItem, *(params->sbDeclare), *(params->sbDefineInit), *(params->sbDefineLayout), addStatement) )) {
								return sl_false;
							}
						}
					}
				} else if (op == OP_SIMULATE) {
					if (!(params->parentResourceItem) && params->resource->layoutType != SAppLayoutType::Window) {
						m_layoutSimulationParams.viewportWidth = params->view->getWidth();
						m_layoutSimulationParams.viewportHeight = params->view->getHeight();
					}
					ListLocker< Ref<SAppLayoutResourceItem> > children(resourceItem->children);
					for (sl_size i = 0; i < children.count; i++) {
						Ref<SAppLayoutResourceItem>& child = children[i];
						if (child.isNotNull()) {
							Ref<View> childView = _simulateLayoutCreateOrLayoutView(params->simulator, child.get(), resourceItem, params->view.get(), params->flagOnLayout);
							if (childView.isNotNull()) {
								if (!(params->flagOnLayout)) {
									params->view->addChild(childView, UIUpdateMode::Init);
								}
							} else {
								return sl_false;
							}
						}
					}
				}
				break;
			default:
				break;
		}

		return sl_true;
		
	}

#define BEGIN_PROCESS_LAYOUT_CONTROL(NAME, VIEWTYPE) \
	sl_bool SAppDocument::_processLayoutResourceControl_##NAME(LayoutControlProcessParams* params) \
	{ \
		String strTab = "\t\t\t"; \
		SAppLayoutResourceItem* resourceItem = params->resourceItem; \
		Ref<XmlElement> element = resourceItem->element; \
		int op = params->op; \
		String name = params->name; \
		sl_bool flagOnLayout = params->flagOnLayout; \
		Ref<SAppLayout##NAME##Attributes> attr; \
		if (op == OP_PARSE) { \
			attr = new SAppLayout##NAME##Attributes; \
			if (attr.isNull()) { \
				_logError(element, _g_sdev_sapp_error_out_of_memory); \
				return sl_false; \
			} \
			resourceItem->attrs##NAME = attr; \
			if (resourceItem->className.isEmpty()) { \
				resourceItem->className = "slib::" #VIEWTYPE; \
			} \
		} else { \
			attr = resourceItem->attrs##NAME; \
			if (op == OP_SIMULATE) { \
				if (!flagOnLayout && params->view.isNull()) { \
					params->view = new VIEWTYPE; \
				} \
			} \
		} \
		VIEWTYPE* view = (VIEWTYPE*)(params->view.get()); \
		SLIB_UNUSED(view)


#define END_PROCESS_LAYOUT_CONTROL \
		return sl_true; \
	}



#define LOG_ERROR_LAYOUT_CONTROL_XML_ATTR(XML, NAME) \
	_logError(XML, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str_##NAME));

#define LAYOUT_CONTROL_PARSE_XML_ATTR(XML, ATTR, NAME) \
	String str_##NAME = XML->getAttribute(#NAME); \
	if (!(ATTR NAME.parse(str_##NAME))) { \
		LOG_ERROR_LAYOUT_CONTROL_XML_ATTR(XML, NAME) \
		return sl_false; \
	}


#define LOG_ERROR_LAYOUT_CONTROL_ATTR(NAME) \
	_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str_##NAME));

#define LAYOUT_CONTROL_PARSE_ATTR(ATTR, NAME) \
	String str_##NAME = resourceItem->getXmlAttribute(#NAME); \
	if (!(ATTR NAME.parse(str_##NAME))) { \
		LOG_ERROR_LAYOUT_CONTROL_ATTR(NAME) \
		return sl_false; \
	}


#define LAYOUT_CONTROL_GENERIC_ATTR(NAME, SETFUNC) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s);%n", strTab, name, attr->NAME.getAccessString())); \
		} \
	} else if (op == OP_SIMULATE) { \
		if (!flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				view->SETFUNC(attr->NAME.value); \
			} \
		} \
	}

#define LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(NAME, SETFUNC) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->NAME.getAccessString())); \
		} \
	} else if (op == OP_SIMULATE) { \
		if (!flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				view->SETFUNC(attr->NAME.value, UIUpdateMode::Init); \
			} \
		} \
	}


#define _LAYOUT_CONTROL_DIMENSION_ATTR(NAME, SETFUNC, CHECKFUNC, TYPE) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
		if (!(attr->NAME.CHECKFUNC())) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			if (attr->NAME.isNeededOnLayoutFunction()) { \
				params->sbDefineLayout->add(String::format("%s%s->" #SETFUNC "(%s);%n", strTab, name, attr->NAME.getAccessString())); \
			} else { \
				params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s);%n", strTab, name, attr->NAME.getAccessString())); \
			} \
		} \
	} else if (op == OP_SIMULATE) { \
		if (flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				view->SETFUNC(_getDimension##TYPE##Value(attr->NAME)); \
			} \
		} \
	}

#define _LAYOUT_CONTROL_DIMENSION_ATTR_NOREDRAW(NAME, SETFUNC, CHECKFUNC, TYPE) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
		if (!(attr->NAME.CHECKFUNC())) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			if (attr->NAME.isNeededOnLayoutFunction()) { \
				params->sbDefineLayout->add(String::format("%s%s->" #SETFUNC "(%s, slib::UIUpdateMode::NoRedraw);%n", strTab, name, attr->NAME.getAccessString())); \
			} else { \
				params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->NAME.getAccessString())); \
			} \
		} \
	} else if (op == OP_SIMULATE) { \
		if (flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				view->SETFUNC(_getDimension##TYPE##Value(attr->NAME), UIUpdateMode::NoRedraw); \
			} \
		} \
	}

#define LAYOUT_CONTROL_INT_DIMENSION_ATTR(NAME, SETFUNC, CHECKFUNC) _LAYOUT_CONTROL_DIMENSION_ATTR(NAME, SETFUNC, CHECKFUNC, Int)
#define LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(NAME, SETFUNC, CHECKFUNC) _LAYOUT_CONTROL_DIMENSION_ATTR_NOREDRAW(NAME, SETFUNC, CHECKFUNC, Int)
#define LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR(NAME, SETFUNC, CHECKFUNC) _LAYOUT_CONTROL_DIMENSION_ATTR(NAME, SETFUNC, CHECKFUNC, Float)
#define LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR_NOREDRAW(NAME, SETFUNC, CHECKFUNC) _LAYOUT_CONTROL_DIMENSION_ATTR_NOREDRAW(NAME, SETFUNC, CHECKFUNC, Float)

#define LAYOUT_CONTROL_MENU_ATTR(NAME, SETFUNC) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			if (!(_checkMenuValueAvailable(attr->NAME, element))) { \
				return sl_false; \
			} \
			params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s);%n", strTab, name, attr->NAME.getAccessString())); \
		} \
	} else if (op == OP_SIMULATE) { \
		if (!flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				if (!(_checkMenuValueAvailable(attr->NAME, element))) { \
					return sl_false; \
				} \
				view->SETFUNC(_getMenuValue(attr->NAME)); \
			} \
		} \
	}


#define LAYOUT_CONTROL_STRING_ATTR(NAME, SETFUNC) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			if (!(_checkStringValueAvailable(attr->NAME, element))) { \
				return sl_false; \
			} \
			params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s);%n", strTab, name, attr->NAME.getAccessString())); \
		} \
	} else if (op == OP_SIMULATE) { \
		if (!flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				if (!(_checkStringValueAvailable(attr->NAME, element))) { \
					return sl_false; \
				} \
				view->SETFUNC(_getStringValue(attr->NAME)); \
			} \
		} \
	}


#define LAYOUT_CONTROL_STRING_ATTR_NOREDRAW(NAME, SETFUNC) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			if (!(_checkStringValueAvailable(attr->NAME, element))) { \
				return sl_false; \
			} \
			params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->NAME.getAccessString())); \
		} \
	} else if (op == OP_SIMULATE) { \
		if (!flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				if (!(_checkStringValueAvailable(attr->NAME, element))) { \
					return sl_false; \
				} \
				view->SETFUNC(_getStringValue(attr->NAME), UIUpdateMode::Init); \
			} \
		} \
	}


#define LAYOUT_CONTROL_DRAWABLE_ATTR(NAME, SETFUNC) \
	if (op == OP_PARSE) { \
		String str = resourceItem->getXmlAttribute(#NAME); \
		if (!(attr->NAME.parse(str))) { \
			_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg(#NAME, str)); \
			return sl_false; \
		} \
	} else if (op == OP_GENERATE_CPP) { \
		if (attr->NAME.flagDefined) { \
			if (!(_checkDrawableValueAvailable(attr->NAME, element))) { \
				return sl_false; \
			} \
			params->sbDefineInit->add(String::format("%s%s->" #SETFUNC "(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->NAME.getAccessString())); \
		} \
	} else if (op == OP_SIMULATE) { \
		if (flagOnLayout) { \
			if (attr->NAME.flagDefined) { \
				if (!(_checkDrawableValueAvailable(attr->NAME, element))) { \
					return sl_false; \
				} \
				view->SETFUNC(_getDrawableValue(attr->NAME), UIUpdateMode::NoRedraw); \
			} \
		} \
	}


#define LAYOUT_CONTROL_PROCESS_SUPER(BASE) \
	String tempAddStatement = params->addStatement; \
	params->addStatement = String::null(); \
	if (!(_processLayoutResourceControl_##BASE(params))) { \
		return sl_false; \
	} \
	params->addStatement = tempAddStatement;

#define LAYOUT_CONTROL_ADD_STATEMENT \
	if (op == OP_GENERATE_CPP) { \
		params->sbDefineInit->add(params->addStatement); \
	}

#define LAYOUT_CONTROL_SET_NATIVE_WIDGET \
	if (op == OP_GENERATE_CPP) { \
		if (resourceItem->attrsView->isNotRequiredNative() || attr->isNotRequiredNative()) { \
			if (!(resourceItem->attrsView->nativeWidget.flagDefined)) { \
				params->sbDefineInit->add(String::format("%s%s->setCreatingNativeWidget(sl_false);%n", strTab, name)); \
			} \
		} \
	} else if (op == OP_SIMULATE) { \
		if (!flagOnLayout) { \
			if (resourceItem->attrsView->isNotRequiredNative() || attr->isNotRequiredNative()) { \
				if (!(resourceItem->attrsView->nativeWidget.flagDefined)) { \
					view->setCreatingNativeWidget(sl_false); \
				} \
			} \
		} \
	}

	BEGIN_PROCESS_LAYOUT_CONTROL(Window, View)
	{
		Window* view = params->window;
		
		LAYOUT_CONTROL_MENU_ATTR(menu, setMenu)
		LAYOUT_CONTROL_STRING_ATTR(title, setTitle)
		if (op == OP_SIMULATE) {
			if (!flagOnLayout) {
				flagOnLayout = sl_true;
				LAYOUT_CONTROL_INT_DIMENSION_ATTR(left, setLeft, checkForWindow)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR(top, setTop, checkForWindow)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR(width, setWidth, checkForWindow)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR(height, setHeight, checkForWindow)
				flagOnLayout = sl_false;
			}
		} else {
			LAYOUT_CONTROL_INT_DIMENSION_ATTR(left, setLeft, checkForWindow)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR(top, setTop, checkForWindow)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR(width, setWidth, checkForWindow)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR(height, setHeight, checkForWindow)		
		}
		LAYOUT_CONTROL_GENERIC_ATTR(minimized, setMinimized)
		LAYOUT_CONTROL_GENERIC_ATTR(maximized, setMaximized)
		LAYOUT_CONTROL_GENERIC_ATTR(visible, setVisible)
		LAYOUT_CONTROL_GENERIC_ATTR(alwaysOnTop, setAlwaysOnTop)
		LAYOUT_CONTROL_GENERIC_ATTR(closeButton, setCloseButtonEnabled)
		LAYOUT_CONTROL_GENERIC_ATTR(minimizeButton, setMinimizeButtonEnabled)
		LAYOUT_CONTROL_GENERIC_ATTR(maximizeButton, setMaximizeButtonEnabled)
		LAYOUT_CONTROL_GENERIC_ATTR(resizable, setResizable)
		LAYOUT_CONTROL_GENERIC_ATTR(alpha, setAlpha)
		LAYOUT_CONTROL_GENERIC_ATTR(transparent, setTransparent)
		
		LAYOUT_CONTROL_GENERIC_ATTR(modal, setModal)
		LAYOUT_CONTROL_GENERIC_ATTR(dialog, setDialog)
		LAYOUT_CONTROL_GENERIC_ATTR(borderless, setBorderless)
		LAYOUT_CONTROL_GENERIC_ATTR(titleBar, setTitleBarVisible)
		LAYOUT_CONTROL_GENERIC_ATTR(fullScreen, setFullScreenOnCreate)
		LAYOUT_CONTROL_GENERIC_ATTR(centerScreen, setCenterScreenOnCreate)
		
		params->name = "m_contentView";
		if (!(_processLayoutResourceControl_View(params))) {
			return sl_false;
		}
		
	}
	END_PROCESS_LAYOUT_CONTROL

	#define SAppLayoutPageAttributes SAppLayoutViewAttributes
	#define attrsPage attrsView
	BEGIN_PROCESS_LAYOUT_CONTROL(Page, ViewPage)
	{
		if (!(_processLayoutResourceControl_View(params))) {
			return sl_false;
		}
		
		if (op == OP_PARSE) {
			if (!(resourceItem->attrsView->width.flagDefined)) {
				resourceItem->attrsView->width.flagDefined = sl_true;
				resourceItem->attrsView->width.amount = 1;
				resourceItem->attrsView->width.unit = SAppDimensionValue::SW;
			}
			if (!(resourceItem->attrsView->height.flagDefined)) {
				resourceItem->attrsView->height.flagDefined = sl_true;
				resourceItem->attrsView->height.amount = 1;
				resourceItem->attrsView->height.unit = SAppDimensionValue::SH;
			}
		}

	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(View, View)
	{
		sl_bool flagView = params->parentResourceItem != sl_null || params->resource->layoutType != SAppLayoutType::Window;
		sl_bool flagRoot = params->parentResourceItem == sl_null;

		if (flagView) {
			LAYOUT_CONTROL_STRING_ATTR(id, setId)
			if (op == OP_PARSE) {
				if (flagRoot) {
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(width, setWidth, checkForRootViewSize)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(height, setHeight, checkForRootViewSize)
				} else {
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(width, setWidth, checkSize)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(height, setHeight, checkSize)
				}
			} else if (op == OP_GENERATE_CPP) {
				if (attr->width.flagDefined) {
					if (attr->width.unit == SAppDimensionValue::FILL) {
						params->sbDefineInit->add(String::format("%s%s->setWidthFilling(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->width.amount));
					} else if (attr->width.unit == SAppDimensionValue::WRAP) {
						params->sbDefineInit->add(String::format("%s%s->setWidthWrapping(slib::UIUpdateMode::Init);%n", strTab, name));
					} else if (attr->width.unit == SAppDimensionValue::WEIGHT) {
						params->sbDefineInit->add(String::format("%s%s->setWidthWeight(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->width.amount));
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(width, setWidth, checkSize)
					}
				}
				if (attr->height.flagDefined) {
					if (attr->height.unit == SAppDimensionValue::FILL) {
						params->sbDefineInit->add(String::format("%s%s->setHeightFilling(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->height.amount));
					} else if (attr->height.unit == SAppDimensionValue::WRAP) {
						params->sbDefineInit->add(String::format("%s%s->setHeightWrapping(slib::UIUpdateMode::Init);%n", strTab, name));
					} else if (attr->height.unit == SAppDimensionValue::WEIGHT) {
						params->sbDefineInit->add(String::format("%s%s->setHeightWeight(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->height.amount));
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(height, setHeight, checkSize)
					}
				}
			} else if (op == OP_SIMULATE) {
				if (attr->width.flagDefined) {
					if (attr->width.unit == SAppDimensionValue::FILL) {
						if (!flagOnLayout) {
							view->setWidthFilling(attr->width.amount, UIUpdateMode::Init);
						}
					} else if (attr->width.unit == SAppDimensionValue::WRAP) {
						if (!flagOnLayout) {
							view->setWidthWrapping(UIUpdateMode::Init);
						}
					} else if (attr->width.unit == SAppDimensionValue::WEIGHT) {
						if (!flagOnLayout) {
							view->setWidthWeight(attr->width.amount, UIUpdateMode::Init);
						}
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(width, setWidth, checkSize)
					}
				}
				if (attr->height.flagDefined) {
					if (attr->height.unit == SAppDimensionValue::FILL) {
						if (!flagOnLayout) {
							view->setHeightFilling(attr->height.amount, UIUpdateMode::Init);
						}
					} else if (attr->height.unit == SAppDimensionValue::WRAP) {
						if (!flagOnLayout) {
							view->setHeightWrapping(UIUpdateMode::Init);
						}
					} else if (attr->height.unit == SAppDimensionValue::WEIGHT) {
						if (!flagOnLayout) {
							view->setHeightWeight(attr->height.amount, UIUpdateMode::Init);
						}
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(height, setHeight, checkSize)
					}
				}
			}
			
			if (flagRoot) {
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(left, setLeft, checkForRootViewPosition)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(top, setTop, checkForRootViewPosition)
			} else {
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(left, setLeft, checkPosition)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(top, setTop, checkPosition)
			}

			if (op == OP_PARSE) {
				
				attr->leftMode = PositionMode::Fixed;
				attr->topMode = PositionMode::Fixed;
				attr->rightMode = PositionMode::Fixed;
				attr->bottomMode = PositionMode::Fixed;
				
				SAppAlignLayoutValue alignLeft;
				LAYOUT_CONTROL_PARSE_ATTR(, alignLeft)
				if (alignLeft.flagDefined) {
					if (alignLeft.flagAlignParent) {
						attr->leftMode = PositionMode::ParentEdge;
					} else {
						if (!flagRoot && params->resource->itemsByName.contains(alignLeft.referingView)) {
							attr->leftMode = PositionMode::OtherStart;
							attr->leftReferingView = alignLeft.referingView;
						} else {
							LOG_ERROR_LAYOUT_CONTROL_ATTR(alignLeft)
							return sl_false;
						}
					}
				}
				SAppNameValue toRightOf;
				LAYOUT_CONTROL_PARSE_ATTR(, toRightOf)
				if (toRightOf.flagDefined) {
					if (!flagRoot && params->resource->itemsByName.contains(toRightOf.value)) {
						attr->leftMode = PositionMode::OtherEnd;
						attr->leftReferingView = toRightOf.value;
					} else {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(toRightOf)
						return sl_false;
					}
				}
				SAppAlignLayoutValue alignTop;
				LAYOUT_CONTROL_PARSE_ATTR(, alignTop)
				if (alignTop.flagDefined) {
					if (alignTop.flagAlignParent) {
						attr->topMode = PositionMode::ParentEdge;
					} else {
						if (!flagRoot && params->resource->itemsByName.contains(alignTop.referingView)) {
							attr->topMode = PositionMode::OtherStart;
							attr->topReferingView = alignTop.referingView;
						} else {
							LOG_ERROR_LAYOUT_CONTROL_ATTR(alignTop)
							return sl_false;
						}
					}
				}
				SAppNameValue below;
				LAYOUT_CONTROL_PARSE_ATTR(, below)
				if (below.flagDefined) {
					if (!flagRoot && params->resource->itemsByName.contains(below.value)) {
						attr->topMode = PositionMode::OtherEnd;
						attr->topReferingView = below.value;
					} else {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(below)
						return sl_false;
					}
				}
				SAppAlignLayoutValue alignRight;
				LAYOUT_CONTROL_PARSE_ATTR(, alignRight)
				if (alignRight.flagDefined) {
					if (alignRight.flagAlignParent) {
						attr->rightMode = PositionMode::ParentEdge;
					} else {
						if (!flagRoot && params->resource->itemsByName.contains(alignRight.referingView)) {
							attr->rightMode = PositionMode::OtherEnd;
							attr->rightReferingView = alignRight.referingView;
						} else {
							LOG_ERROR_LAYOUT_CONTROL_ATTR(alignRight)
							return sl_false;
						}
					}
				}
				SAppNameValue toLeftOf;
				LAYOUT_CONTROL_PARSE_ATTR(, toLeftOf)
				if (toLeftOf.flagDefined) {
					if (!flagRoot && params->resource->itemsByName.contains(toLeftOf.value)) {
						attr->rightMode = PositionMode::OtherStart;
						attr->rightReferingView = toLeftOf.value;
					} else {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(toLeftOf)
						return sl_false;
					}
				}
				SAppAlignLayoutValue alignBottom;
				LAYOUT_CONTROL_PARSE_ATTR(, alignBottom)
				if (alignBottom.flagDefined) {
					if (alignBottom.flagAlignParent) {
						attr->bottomMode = PositionMode::ParentEdge;
					} else {
						if (!flagRoot && params->resource->itemsByName.contains(alignBottom.referingView)) {
							attr->bottomMode = PositionMode::OtherEnd;
							attr->bottomReferingView = alignBottom.referingView;
						} else {
							LOG_ERROR_LAYOUT_CONTROL_ATTR(alignBottom)
							return sl_false;
						}
					}
				}
				SAppNameValue above;
				LAYOUT_CONTROL_PARSE_ATTR(, above)
				if (above.flagDefined) {
					if (!flagRoot && params->resource->itemsByName.contains(above.value)) {
						attr->bottomMode = PositionMode::OtherStart;
						attr->bottomReferingView = above.value;
					} else {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(above)
						return sl_false;
					}
				}
				SAppAlignLayoutValue centerHorizontal;
				LAYOUT_CONTROL_PARSE_ATTR(, centerHorizontal)
				if (centerHorizontal.flagDefined) {
					if (centerHorizontal.flagAlignParent) {
						attr->leftMode = PositionMode::CenterInParent;
					} else {
						if (!flagRoot && params->resource->itemsByName.contains(centerHorizontal.referingView)) {
							attr->leftMode = PositionMode::CenterInOther;
							attr->leftReferingView = centerHorizontal.referingView;
						} else {
							LOG_ERROR_LAYOUT_CONTROL_ATTR(centerHorizontal)
							return sl_false;
						}
					}
				}
				SAppAlignLayoutValue centerVertical;
				LAYOUT_CONTROL_PARSE_ATTR(, centerVertical)
				if (centerVertical.flagDefined) {
					if (centerVertical.flagAlignParent) {
						attr->topMode = PositionMode::CenterInParent;
					} else {
						if (!flagRoot && params->resource->itemsByName.contains(centerVertical.referingView)) {
							attr->topMode = PositionMode::CenterInOther;
							attr->topReferingView = centerVertical.referingView;
						} else {
							LOG_ERROR_LAYOUT_CONTROL_ATTR(centerVertical)
							return sl_false;
						}
					}
				}
				SAppAlignLayoutValue alignCenter;
				LAYOUT_CONTROL_PARSE_ATTR(, alignCenter)
				if (alignCenter.flagDefined) {
					if (alignCenter.flagAlignParent) {
						attr->leftMode = PositionMode::CenterInParent;
						attr->topMode = PositionMode::CenterInParent;
					} else {
						if (!flagRoot && params->resource->itemsByName.contains(alignCenter.referingView)) {
							attr->leftMode = PositionMode::CenterInOther;
							attr->topMode = PositionMode::CenterInOther;
							attr->leftReferingView = alignCenter.referingView;
						} else {
							LOG_ERROR_LAYOUT_CONTROL_ATTR(alignCenter)
							return sl_false;
						}
					}
				}
			} else if (op == OP_GENERATE_CPP) {
				if (attr->leftMode == PositionMode::CenterInParent) {
					params->sbDefineInit->add(String::format("%s%s->setCenterHorizontal(slib::UIUpdateMode::Init);%n", strTab, name, attr->leftReferingView));
				} else if (attr->leftMode == PositionMode::CenterInOther) {
					params->sbDefineInit->add(String::format("%s%s->setAlignCenterHorizontal(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->leftReferingView));
				} else {
					if (attr->leftMode == PositionMode::ParentEdge) {
						params->sbDefineInit->add(String::format("%s%s->setAlignParentLeft(slib::UIUpdateMode::Init);%n", strTab, name));
					} else if (attr->leftMode == PositionMode::OtherStart) {
						params->sbDefineInit->add(String::format("%s%s->setAlignLeft(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->leftReferingView));
					} else if (attr->leftMode == PositionMode::OtherEnd) {
						params->sbDefineInit->add(String::format("%s%s->setRightOf(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->leftReferingView));
					}
					if (attr->rightMode == PositionMode::ParentEdge) {
						params->sbDefineInit->add(String::format("%s%s->setAlignParentRight(slib::UIUpdateMode::Init);%n", strTab, name));
					} else if (attr->rightMode == PositionMode::OtherStart) {
						params->sbDefineInit->add(String::format("%s%s->setLeftOf(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->rightReferingView));
					} else if (attr->rightMode == PositionMode::OtherEnd) {
						params->sbDefineInit->add(String::format("%s%s->setAlignRight(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->rightReferingView));
					}
				}
				
				if (attr->topMode == PositionMode::CenterInParent) {
					params->sbDefineInit->add(String::format("%s%s->setCenterVertical(slib::UIUpdateMode::Init);%n", strTab, name, attr->topReferingView));
				} else if (attr->topMode == PositionMode::CenterInOther) {
					params->sbDefineInit->add(String::format("%s%s->setAlignCenterVertical(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->topReferingView));
				} else {
					if (attr->topMode == PositionMode::ParentEdge) {
						params->sbDefineInit->add(String::format("%s%s->setAlignParentTop(slib::UIUpdateMode::Init);%n", strTab, name));
					} else if (attr->topMode == PositionMode::OtherStart) {
						params->sbDefineInit->add(String::format("%s%s->setAlignTop(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->topReferingView));
					} else if (attr->topMode == PositionMode::OtherEnd) {
						params->sbDefineInit->add(String::format("%s%s->setBelow(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->topReferingView));
					}
					if (attr->bottomMode == PositionMode::ParentEdge) {
						params->sbDefineInit->add(String::format("%s%s->setAlignParentBottom(slib::UIUpdateMode::Init);%n", strTab, name));
					} else if (attr->bottomMode == PositionMode::OtherStart) {
						params->sbDefineInit->add(String::format("%s%s->setAbove(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->bottomReferingView));
					} else if (attr->bottomMode == PositionMode::OtherEnd) {
						params->sbDefineInit->add(String::format("%s%s->setAlignBottom(%s, slib::UIUpdateMode::Init);%n", strTab, name, attr->bottomReferingView));
					}
				}
			} else if (op == OP_SIMULATE) {
				if (attr->leftMode == PositionMode::CenterInParent) {
					if (!flagOnLayout) {
						view->setCenterHorizontal(UIUpdateMode::Init);
					}
				} else if (attr->leftMode == PositionMode::CenterInOther) {
					if (!flagOnLayout) {
						view->setAlignCenterHorizontal(params->simulator->getViewByName(attr->leftReferingView), UIUpdateMode::Init);
					}
				} else {
					if (attr->leftMode == PositionMode::ParentEdge) {
						if (!flagOnLayout) {
							view->setAlignParentLeft(UIUpdateMode::Init);
						}
					} else if (attr->leftMode == PositionMode::OtherStart) {
						if (!flagOnLayout) {
							view->setAlignLeft(params->simulator->getViewByName(attr->leftReferingView), UIUpdateMode::Init);
						}
					} else if (attr->leftMode == PositionMode::OtherEnd) {
						if (!flagOnLayout) {
							view->setRightOf(params->simulator->getViewByName(attr->leftReferingView), UIUpdateMode::Init);
						}
					}
					if (attr->rightMode == PositionMode::ParentEdge) {
						if (!flagOnLayout) {
							view->setAlignParentRight(UIUpdateMode::Init);
						}
					} else if (attr->rightMode == PositionMode::OtherStart) {
						if (!flagOnLayout) {
							view->setLeftOf(params->simulator->getViewByName(attr->rightReferingView), UIUpdateMode::Init);
						}
					} else if (attr->rightMode == PositionMode::OtherEnd) {
						if (!flagOnLayout) {
							view->setAlignRight(params->simulator->getViewByName(attr->rightReferingView), UIUpdateMode::Init);
						}
					}
				}
				
				if (attr->topMode == PositionMode::CenterInParent) {
					if (!flagOnLayout) {
						view->setCenterVertical(UIUpdateMode::Init);
					}
				} else if (attr->topMode == PositionMode::CenterInOther) {
					if (!flagOnLayout) {
						view->setAlignCenterVertical(params->simulator->getViewByName(attr->topReferingView), UIUpdateMode::Init);
					}
				} else {
					if (attr->topMode == PositionMode::ParentEdge) {
						if (!flagOnLayout) {
							view->setAlignParentTop(UIUpdateMode::Init);
						}
					} else if (attr->topMode == PositionMode::OtherStart) {
						if (!flagOnLayout) {
							view->setAlignTop(params->simulator->getViewByName(attr->topReferingView), UIUpdateMode::Init);
						}
					} else if (attr->topMode == PositionMode::OtherEnd) {
						if (!flagOnLayout) {
							view->setBelow(params->simulator->getViewByName(attr->topReferingView), UIUpdateMode::Init);
						}
					}
					if (attr->bottomMode == PositionMode::ParentEdge) {
						if (!flagOnLayout) {
							view->setAlignParentBottom(UIUpdateMode::Init);
						}
					} else if (attr->bottomMode == PositionMode::OtherStart) {
						if (!flagOnLayout) {
							view->setAbove(params->simulator->getViewByName(attr->bottomReferingView), UIUpdateMode::Init);
						}
					} else if (attr->bottomMode == PositionMode::OtherEnd) {
						if (!flagOnLayout) {
							view->setAlignBottom(params->simulator->getViewByName(attr->bottomReferingView), UIUpdateMode::Init);
						}
					}
				}
			}
			
			if (flagRoot) {
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(minWidth, setMinimumWidth, checkForRootViewScalarSize)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(maxWidth, setMaximumWidth, checkForRootViewScalarSize)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(minHeight, setMinimumHeight, checkForRootViewScalarSize)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(maxHeight, setMaximumHeight, checkForRootViewScalarSize)
			} else {
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(minWidth, setMinimumWidth, checkScalarSize)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(maxWidth, setMaximumWidth, checkScalarSize)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(minHeight, setMinimumHeight, checkScalarSize)
				LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(maxHeight, setMaximumHeight, checkScalarSize)
			}
			
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(aspectRatio, setAspectRatio)
			if (op == OP_GENERATE_CPP) {
				if (attr->aspectRatio.flagDefined) {
					if (attr->width.flagDefined) {
						if (!(attr->height.flagDefined)) {
							params->sbDefineInit->add(String::format("%s%s->setAspectRatioMode(slib::AspectRatioMode::AdjustHeight, slib::UIUpdateMode::Init);%n", strTab, name));
						}
					} else {
						if (attr->height.flagDefined) {
							params->sbDefineInit->add(String::format("%s%s->setAspectRatioMode(slib::AspectRatioMode::AdjustWidth, slib::UIUpdateMode::Init);%n", strTab, name));
						}
					}
				}
			} else if (op == OP_SIMULATE) {
				if (attr->aspectRatio.flagDefined) {
					if (!flagOnLayout) {
						if (attr->width.flagDefined) {
							if (!(attr->height.flagDefined)) {
								view->setAspectRatioMode(AspectRatioMode::AdjustHeight, slib::UIUpdateMode::Init);
							}
						} else {
							if (attr->height.flagDefined) {
								view->setAspectRatioMode(AspectRatioMode::AdjustWidth, slib::UIUpdateMode::Init);
							}
						}
					}
				}
			}

			if (op == OP_PARSE) {
				if (flagRoot) {
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginLeft, setMarginLeft, checkForRootViewMargin)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginTop, setMarginTop, checkForRootViewMargin)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginRight, setMarginRight, checkForRootViewMargin)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginBottom, setMarginBottom, checkForRootViewMargin)
				} else {
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginLeft, setMarginLeft, checkMargin)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginTop, setMarginTop, checkMargin)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginRight, setMarginRight, checkMargin)
					LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginBottom, setMarginBottom, checkMargin)
				}
				SAppDimensionValue margin;
				LAYOUT_CONTROL_PARSE_ATTR(, margin)
				if (flagRoot) {
					if (!(margin.checkMargin())) {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(margin)
						return sl_false;
					}
				} else {
					if (!(margin.checkForRootViewMargin())) {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(margin)
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
			} else if (op == OP_GENERATE_CPP) {
				if (attr->marginLeft.flagDefined) {
					if (attr->marginLeft.unit == SAppDimensionValue::WEIGHT) {
						params->sbDefineInit->add(String::format("%s%s->setRelativeMarginLeft(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->marginLeft.amount));
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginLeft, setMarginLeft, checkMargin)
					}
				}
				if (attr->marginTop.flagDefined) {
					if (attr->marginTop.unit == SAppDimensionValue::WEIGHT) {
						params->sbDefineInit->add(String::format("%s%s->setRelativeMarginTop(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->marginTop.amount));
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginTop, setMarginTop, checkMargin)
					}
				}
				if (attr->marginRight.flagDefined) {
					if (attr->marginRight.unit == SAppDimensionValue::WEIGHT) {
						params->sbDefineInit->add(String::format("%s%s->setRelativeMarginRight(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->marginRight.amount));
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginRight, setMarginRight, checkMargin)
					}
				}
				if (attr->marginBottom.flagDefined) {
					if (attr->marginBottom.unit == SAppDimensionValue::WEIGHT) {
						params->sbDefineInit->add(String::format("%s%s->setRelativeMarginBottom(%ff, slib::UIUpdateMode::Init);%n", strTab, name, attr->marginBottom.amount));
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginBottom, setMarginBottom, checkMargin)
					}
				}
			} else if (op == OP_SIMULATE) {
				if (attr->marginLeft.flagDefined) {
					if (attr->marginLeft.unit == SAppDimensionValue::WEIGHT) {
						if (!flagOnLayout) {
							view->setRelativeMarginLeft(attr->marginLeft.amount, UIUpdateMode::Init);
						}
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginLeft, setMarginLeft, checkMargin)
					}
				}
				if (attr->marginTop.flagDefined) {
					if (attr->marginTop.unit == SAppDimensionValue::WEIGHT) {
						if (!flagOnLayout) {
							view->setRelativeMarginTop(attr->marginTop.amount, UIUpdateMode::Init);
						}
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginTop, setMarginTop, checkMargin)
					}
				}
				if (attr->marginRight.flagDefined) {
					if (attr->marginRight.unit == SAppDimensionValue::WEIGHT) {
						if (!flagOnLayout) {
							view->setRelativeMarginRight(attr->marginRight.amount, UIUpdateMode::Init);
						}
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginRight, setMarginRight, checkMargin)
					}
				}
				if (attr->marginBottom.flagDefined) {
					if (attr->marginBottom.unit == SAppDimensionValue::WEIGHT) {
						if (!flagOnLayout) {
							view->setRelativeMarginBottom(attr->marginBottom.amount, UIUpdateMode::Init);
						}
					} else {
						LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(marginBottom, setMarginBottom, checkMargin)
					}
				}
			}
		}
		
		if (flagRoot) {
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingLeft, setPaddingLeft, checkForRootViewPosition)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingTop, setPaddingTop, checkForRootViewPosition)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingRight, setPaddingRight, checkForRootViewPosition)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingBottom, setPaddingBottom, checkForRootViewPosition)
		} else {
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingLeft, setPaddingLeft, checkPosition)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingTop, setPaddingTop, checkPosition)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingRight, setPaddingRight, checkPosition)
			LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(paddingBottom, setPaddingBottom, checkPosition)
		}
		if (op == OP_PARSE) {
			SAppDimensionValue padding;
			LAYOUT_CONTROL_PARSE_ATTR(, padding)
			if (flagRoot) {
				if (!(padding.checkForRootViewPosition())) {
					LOG_ERROR_LAYOUT_CONTROL_ATTR(padding)
					return sl_false;
				}
			} else {
				if (!(padding.checkPosition())) {
					LOG_ERROR_LAYOUT_CONTROL_ATTR(padding)
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
		
		if (flagView) {
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(visibility, setVisibility)
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(visible, setVisible)
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(enabled, setEnabled)
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(clipping, setClipping)
		}
		LAYOUT_CONTROL_DRAWABLE_ATTR(background, setBackground)
		LAYOUT_CONTROL_DRAWABLE_ATTR(pressedBackground, setPressedBackground)
		LAYOUT_CONTROL_DRAWABLE_ATTR(hoverBackground, setHoverBackground)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(backgroundScale, setBackgroundScaleMode)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(backgroundAlign, setBackgroundAlignment)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(backgroundColor, setBackgroundColor)
		
		if (flagView) {
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(border, setBorder)
			if (op == OP_PARSE) {
				if (flagRoot) {
					LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR_NOREDRAW(borderWidth, setBorderWidth, checkForRootViewScalarSize)
				} else {
					LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR_NOREDRAW(borderWidth, setBorderWidth, checkScalarSize)
				}
				LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderColor, setBorderColor)
				LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderStyle, setBorderStyle)
			} else if (op == OP_GENERATE_CPP){
				if (attr->borderWidth.flagDefined) {
					if (Math::isAlmostZero(attr->borderWidth.amount)) {
						params->sbDefineInit->add(String::format("%s%s->setBorder(slib::Ref<slib::Pen>::null(), slib::UIUpdateMode::Init);%n", strTab, name));
					} else {
						if (attr->borderColor.flagDefined && attr->borderStyle.flagDefined) {
							if (attr->borderWidth.isNeededOnLayoutFunction()) {
								params->sbDefineLayout->add(String::format("%s%s->setBorder(slib::Pen::create(%s, %s, %s), slib::UIUpdateMode::NoRedraw);%n", strTab, name, attr->borderStyle.getAccessString(), attr->borderWidth.getAccessString(), attr->borderColor.getAccessString()));
							} else {
								params->sbDefineInit->add(String::format("%s%s->setBorder(slib::Pen::create(%s, %s, %s), slib::UIUpdateMode::Init);%n", strTab, name, attr->borderStyle.getAccessString(), attr->borderWidth.getAccessString(), attr->borderColor.getAccessString()));
							}
						} else {
							LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR_NOREDRAW(borderWidth, setBorderWidth, checkScalarSize)
							LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderColor, setBorderColor)
							LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderStyle, setBorderStyle)
						}
					}
				} else {
					LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderColor, setBorderColor)
					LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderStyle, setBorderStyle)
				}
			} else if (op == OP_SIMULATE) {
				if (attr->borderWidth.flagDefined) {
					if (Math::isAlmostZero(attr->borderWidth.amount)) {
						if (!flagOnLayout) {
							view->setBorder(Ref<Pen>::null(), UIUpdateMode::Init);
						}
					} else {
						if (attr->borderColor.flagDefined && attr->borderStyle.flagDefined) {
							if (flagOnLayout) {
								view->setBorder(Pen::create(attr->borderStyle.value, _getDimensionFloatValue(attr->borderWidth), attr->borderColor.value), UIUpdateMode::NoRedraw);
							}
						} else {
							LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR_NOREDRAW(borderWidth, setBorderWidth, checkScalarSize)
							LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderColor, setBorderColor)
							LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderStyle, setBorderStyle)
						}
					}
				} else {
					LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderColor, setBorderColor)
					LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(borderStyle, setBorderStyle)
				}
			}
		}
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(drawing, setDrawing)

		if (flagView) {
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(boundShape, setBoundShape)
			if (flagRoot) {
				LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR_NOREDRAW(boundRadiusX, setRoundRectBoundShapeRadiusX, checkForRootViewScalarSize)
			} else {
				LAYOUT_CONTROL_FLOAT_DIMENSION_ATTR_NOREDRAW(boundRadiusY, setRoundRectBoundShapeRadiusY, checkScalarSize)
			}
			if (op == OP_PARSE) {
				SAppDimensionFloatValue boundRadius;
				LAYOUT_CONTROL_PARSE_ATTR(, boundRadius)
				if (flagRoot) {
					if (!(boundRadius.checkForRootViewScalarSize())) {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(boundRadius)
						return sl_false;
					}
				} else {
					if (!(boundRadius.checkScalarSize())) {
						LOG_ERROR_LAYOUT_CONTROL_ATTR(boundRadius)
						return sl_false;
					}
				}
				if (boundRadius.flagDefined) {
					if (!(attr->boundRadiusX.flagDefined)) {
						attr->boundRadiusX = boundRadius;
					}
					if (!(attr->boundRadiusY.flagDefined)) {
						attr->boundRadiusY = boundRadius;
					}
				}
			}
		}

		if (op == OP_PARSE) {
			LAYOUT_CONTROL_PARSE_ATTR(attr->, fontFamily)
			if (attr->fontFamily.flagDefined) {
				attr->finalFontFamily = attr->fontFamily;
			} else {
				if (!flagRoot) {
					attr->finalFontFamily = params->parentResourceItem->attrsView->finalFontFamily;
				}
			}
			LAYOUT_CONTROL_PARSE_ATTR(attr->, fontSize)
			if (flagRoot) {
				if (!(attr->fontSize.checkForRootViewPosition())) {
					LOG_ERROR_LAYOUT_CONTROL_ATTR(fontSize)
					return sl_false;
				}
			} else {
				if (!(attr->fontSize.checkPosition())) {
					LOG_ERROR_LAYOUT_CONTROL_ATTR(fontSize)
					return sl_false;
				}
			}
			if (attr->fontSize.flagDefined) {
				attr->finalFontSize = attr->fontSize;
			} else {
				if (!flagRoot) {
					attr->finalFontSize = params->parentResourceItem->attrsView->finalFontSize;
				}
			}
			LAYOUT_CONTROL_PARSE_ATTR(attr->, fontBold)
			if (attr->fontBold.flagDefined) {
				attr->finalFontBold = attr->fontBold.value;
			} else {
				if (!flagRoot) {
					attr->finalFontBold = params->parentResourceItem->attrsView->finalFontBold;
				}
			}
			LAYOUT_CONTROL_PARSE_ATTR(attr->, fontItalic)
			if (attr->fontItalic.flagDefined) {
				attr->finalFontItalic = attr->fontItalic.value;
			} else {
				if (!flagRoot) {
					attr->finalFontItalic = params->parentResourceItem->attrsView->finalFontItalic;
				}
			}
			LAYOUT_CONTROL_PARSE_ATTR(attr->, fontUnderline)
			if (attr->fontUnderline.flagDefined) {
				attr->finalFontUnderline = attr->fontUnderline.value;
			} else {
				if (!flagRoot) {
					attr->finalFontUnderline = params->parentResourceItem->attrsView->finalFontUnderline;
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			if (!(_checkStringValueAvailable(attr->fontFamily, element))) {
				return sl_false;
			}
			if (attr->fontFamily.flagDefined || attr->fontSize.flagDefined || attr->fontBold.flagDefined || attr->fontItalic.flagDefined || attr->fontUnderline.flagDefined) {
				String fontSize;
				if (attr->finalFontSize.flagDefined) {
					fontSize = attr->finalFontSize.getAccessString();
				} else {
					fontSize = "slib::UI::getDefaultFontSize()";
				}
				StringBuffer* sb;
				String strUpdateMode;
				if (attr->finalFontSize.isNeededOnLayoutFunction()) {
					sb = params->sbDefineLayout;
					strUpdateMode = "slib::UIUpdateMode::NoRedraw";
				} else {
					sb = params->sbDefineInit;
					strUpdateMode = "slib::UIUpdateMode::Init";
				}
				if (attr->finalFontFamily.flagDefined) {
					sb->add(String::format("%s%s->setFont(%s, ", strTab, name, attr->finalFontFamily.getAccessString()));
				} else {
					sb->add(String::format("%s%s->setFontAttributes(", strTab, name));
				}
				sb->add(String::format("%s, %s, %s, %s, %s);%n", fontSize, attr->finalFontBold?"sl_true":"sl_false", attr->finalFontItalic?"sl_true":"sl_false", attr->finalFontUnderline?"sl_true":"sl_false", strUpdateMode));
			}
		} else if (op == OP_SIMULATE) {
			if (flagOnLayout) {
				if (!(_checkStringValueAvailable(attr->fontFamily, element))) {
					return sl_false;
				}
				if (attr->fontFamily.flagDefined || attr->fontSize.flagDefined || attr->fontBold.flagDefined || attr->fontItalic.flagDefined || attr->fontUnderline.flagDefined) {
					sl_real fontSize;
					if (attr->finalFontSize.flagDefined) {
						fontSize = _getDimensionFloatValue(attr->finalFontSize);
					} else {
						fontSize = UI::getDefaultFontSize();
					}
					if (attr->finalFontFamily.flagDefined) {
						view->setFont(_getStringValue(attr->finalFontFamily), fontSize, attr->finalFontBold, attr->finalFontItalic, attr->finalFontUnderline, UIUpdateMode::NoRedraw);
					} else {
						view->setFontAttributes(fontSize, attr->finalFontBold, attr->finalFontItalic, attr->finalFontUnderline, UIUpdateMode::NoRedraw);
					}
				}
			}
		}
		
		if (flagView) {
			LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(alpha, setAlpha)
		}
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(opaque, setOpaque)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(layer, setLayer)
		
		if (op == OP_PARSE) {
			LAYOUT_CONTROL_PARSE_ATTR(attr->, scrolling)
			if (!(attr->scrolling.flagDefined)) {
				if (resourceItem->element->getName() == "hscroll") {
					attr->scrolling.flagDefined = sl_true;
					attr->scrolling.horizontal = sl_true;
					attr->scrolling.vertical = sl_false;
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			if (attr->scrolling.flagDefined) {
				params->sbDefineInit->add(String::format("%s%s->setHorizontalScrolling(%s);%n", strTab, name, (attr->scrolling.horizontal?"sl_true":"sl_false")));
				params->sbDefineInit->add(String::format("%s%s->setVerticalScrolling(%s);%n", strTab, name, (attr->scrolling.vertical?"sl_true":"sl_false")));
			}
		} else if (op == OP_SIMULATE) {
			if (!flagOnLayout) {
				if (attr->scrolling.flagDefined) {
					view->setHorizontalScrolling(attr->scrolling.horizontal?sl_true:sl_false);
					view->setVerticalScrolling(attr->scrolling.vertical?sl_true:sl_false);
				}
			}
		}
		
		LAYOUT_CONTROL_GENERIC_ATTR(paging, setPaging)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR(pageWidth, setPageWidth, checkScalarSize)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR(pageHeight, setPageHeight, checkScalarSize)
		
		if (op == OP_PARSE) {
			LAYOUT_CONTROL_PARSE_ATTR(attr->, scrollBars)
		} else if (op == OP_GENERATE_CPP) {
			if (attr->scrollBars.flagDefined) {
				params->sbDefineInit->add(String::format("%s%s->setScrollBarsVisible(%s, %s, slib::UIUpdateMode::Init);%n", strTab, name, attr->scrollBars.horizontalScrollBar ? "sl_true" : "sl_false", attr->scrollBars.verticalScrollBar ? "sl_true" : "sl_false"));
			}
		} else if (op == OP_SIMULATE) {
			if (attr->scrollBars.flagDefined) {
				view->setScrollBarsVisible(attr->scrollBars.horizontalScrollBar, attr->scrollBars.verticalScrollBar, UIUpdateMode::NoRedraw);
			}
		}
		LAYOUT_CONTROL_GENERIC_ATTR(scrollingByMouse, setContentScrollingByMouse)
		LAYOUT_CONTROL_GENERIC_ATTR(scrollingByTouch, setContentScrollingByTouch)
		LAYOUT_CONTROL_GENERIC_ATTR(scrollingByMouseWheel, setContentScrollingByMouseWheel)
		LAYOUT_CONTROL_GENERIC_ATTR(scrollingByKeyboard, setContentScrollingByKeyboard)
		
		LAYOUT_CONTROL_GENERIC_ATTR(touchMultipleChildren, setTouchMultipleChildren)
		if (flagView) {
			LAYOUT_CONTROL_GENERIC_ATTR(tabStop, setProcessingTabStop)
		}
		
		if (flagView) {
			LAYOUT_CONTROL_GENERIC_ATTR(instance, setCreatingInstance)
			LAYOUT_CONTROL_GENERIC_ATTR(nativeWidget, setCreatingNativeWidget)
			LAYOUT_CONTROL_GENERIC_ATTR(hardwareLayer, setHardwareLayer)
		}
		LAYOUT_CONTROL_GENERIC_ATTR(childInstances, setCreatingChildInstances)
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


#define SAppLayoutViewGroupAttributes SAppLayoutViewAttributes
#define attrsViewGroup attrsView
	BEGIN_PROCESS_LAYOUT_CONTROL(ViewGroup, ViewGroup)
	{
		if (!(_processLayoutResourceControl_View(params))) {
			return sl_false;
		}
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Import, SAppLayoutImportView)
	{
		if (op == OP_PARSE) {
			attr->layout = resourceItem->getXmlAttribute("layout");
			if (attr->layout.isEmpty()) {
				_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("layout", attr->layout));
				return sl_false;
			}
			resourceItem->className = attr->layout;
		} else if (op == OP_GENERATE_CPP) {
			Ref<SAppLayoutResource> layoutImport;
			m_layouts.get(attr->layout, &layoutImport);
			if (layoutImport.isNull()) {
				_logError(element, _g_sdev_sapp_error_layout_not_found.arg(attr->layout));
				return sl_false;
			}
			if (layoutImport->layoutType != SAppLayoutType::View && layoutImport->layoutType != SAppLayoutType::Page) {
				_logError(element, _g_sdev_sapp_error_layout_is_not_view.arg(attr->layout));
				return sl_false;
			}
		} else if (op == OP_SIMULATE) {
			Ref<SAppLayoutResource> layoutImport;
			m_layouts.get(attr->layout, &layoutImport);
			if (layoutImport.isNull()) {
				_logError(element, _g_sdev_sapp_error_layout_not_found.arg(attr->layout));
				return sl_false;
			}
			if (layoutImport->layoutType != SAppLayoutType::View && layoutImport->layoutType != SAppLayoutType::Page) {
				_logError(element, _g_sdev_sapp_error_layout_is_not_view.arg(attr->layout));
				return sl_false;
			}
			if (!flagOnLayout) {
				Ref<SAppLayoutImportView> _view = new SAppLayoutImportView;
				if (_view.isNotNull()) {
					_view->init(params->simulator, layoutImport.get());
				} else {
					return sl_false;
				}
				params->view = _view;
				view = _view.get();
			} else {
				if (!view) {
					return sl_false;
				}
			}
		}
		
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
		if (op == OP_SIMULATE) {
			if (flagOnLayout) {
				view->layoutViews(view->getWidth(), view->getHeight());
			}
		}
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Button, Button)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_STRING_ATTR_NOREDRAW(text, setText)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(multiLine, setMultiLine)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(defaultButton, setDefaultButton)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textColor, setTextColor)
		LAYOUT_CONTROL_DRAWABLE_ATTR(icon, setIcon)
		
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconWidth, setIconWidth, checkScalarSize)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconHeight, setIconHeight, checkScalarSize)
		if (op == OP_PARSE) {
			SAppDimensionValue iconSize;
			LAYOUT_CONTROL_PARSE_ATTR(, iconSize)
			if (!(iconSize.checkScalarSize())) {
				LOG_ERROR_LAYOUT_CONTROL_ATTR(iconSize)
				return sl_false;
			}
			if (iconSize.flagDefined) {
				if (!(attr->iconWidth.flagDefined)) {
					attr->iconWidth = iconSize;
				}
				if (!(attr->iconHeight.flagDefined)) {
					attr->iconHeight = iconSize;
				}
			}
		}
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(gravity, setGravity)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(iconAlign, setIconAlignment)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textAlign, setTextAlignment)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textBeforeIcon, setTextBeforeIcon)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(orientation, setLayoutOrientation)
		
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconMarginLeft, setIconMarginLeft, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconMarginTop, setIconMarginTop, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconMarginRight, setIconMarginRight, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconMarginBottom, setIconMarginBottom, checkPosition)
		if (op == OP_PARSE) {
			SAppDimensionValue iconMargin;
			LAYOUT_CONTROL_PARSE_ATTR(, iconMargin)
			if (!(iconMargin.checkPosition())) {
				LOG_ERROR_LAYOUT_CONTROL_ATTR(iconMargin)
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
		}
		
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(textMarginLeft, setTextMarginLeft, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(textMarginTop, setTextMarginTop, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(textMarginRight, setTextMarginRight, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(textMarginBottom, setTextMarginBottom, checkPosition)
		if (op == OP_PARSE) {
			SAppDimensionValue textMargin;
			LAYOUT_CONTROL_PARSE_ATTR(, textMargin)
			if (!(textMargin.checkPosition())) {
				LOG_ERROR_LAYOUT_CONTROL_ATTR(textMargin)
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
		}
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(defaultColorFilter, setUsingDefaultColorFilter)
		
		String strStates[] = {"Normal", "Hover", "Pressed", "Disabled"};
		ButtonState states[] = {ButtonState::Normal, ButtonState::Hover, ButtonState::Pressed, ButtonState::Disabled};
		for (sl_uint32 i = 0; i < SLIB_SAPP_LAYOUT_BUTTON_CATEGORY_MAX; i++) {
			SAppLayoutButtonCategory& category = attr->categories[i];
			for (sl_uint32 k = 0; k < (sl_uint32)(ButtonState::Count); k++) {
				if (op == OP_PARSE) {
					String suffix;
					if (i > 0) {
						suffix = String::format("%s%d", strStates[k], i);
					} else {
						suffix = strStates[k];
					}
					String _strTextColor = resourceItem->getXmlAttribute("textColor" + suffix);
					if (!(category.textColor[k].parse(_strTextColor))) {
						_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("textColor" + suffix, _strTextColor));
						return sl_false;
					}
					String _strIcon = resourceItem->getXmlAttribute("icon" + suffix);
					if (!(category.icon[k].parse(_strIcon))) {
						_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("icon" + suffix, _strIcon));
						return sl_false;
					}
					String _strBackground = resourceItem->getXmlAttribute("background" + suffix);
					if (!(category.background[k].parse(_strBackground))) {
						_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("background" + suffix, _strBackground));
						return sl_false;
					}
					String _strBorderStyle = resourceItem->getXmlAttribute("borderStyle" + suffix);
					if (!(category.borderStyle[k].parse(_strBorderStyle))) {
						_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderStyle" + suffix, _strBorderStyle));
						return sl_false;
					}
					String _strBorderWidth = resourceItem->getXmlAttribute("borderWidth" + suffix);
					if (!(category.borderWidth[k].parse(_strBorderWidth))) {
						_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderWidth" + suffix, _strBorderWidth));
						return sl_false;
					}
					if (!(category.borderWidth[k].checkScalarSize())) {
						_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderWidth" + suffix, _strBorderWidth));
						return sl_false;
					}
					String _strBorderColor = resourceItem->getXmlAttribute("borderColor" + suffix);
					if (!(category.borderColor[k].parse(_strBorderColor))) {
						_logError(element, _g_sdev_sapp_error_resource_layout_attribute_invalid.arg("borderColor" + suffix, _strBorderColor));
						return sl_false;
					}
				} else if (op == OP_GENERATE_CPP) {
					if (category.textColor[k].flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setTextColor(%s, slib::ButtonState::%s, %d, slib::UIUpdateMode::Init);%n", strTab, name, category.textColor[k].getAccessString(), strStates[k], i));
					}
					if (category.icon[k].flagDefined) {
						if (!(_checkDrawableValueAvailable(category.icon[k], resourceItem->element))) {
							return sl_false;
						}
						params->sbDefineInit->add(String::format("%s%s->setIcon(%s, slib::ButtonState::%s, %d, slib::UIUpdateMode::Init);%n", strTab, name, category.icon[k].getAccessString(), strStates[k], i));
					}
					if (category.background[k].flagDefined) {
						if (!(_checkDrawableValueAvailable(category.background[k], resourceItem->element))) {
							return sl_false;
						}
						params->sbDefineInit->add(String::format("%s%s->setBackground(%s, slib::ButtonState::%s, %d, slib::UIUpdateMode::Init);%n", strTab, name, category.background[k].getAccessString(), strStates[k], i));
					}
					if (category.borderWidth[k].flagDefined || category.borderColor[k].flagDefined || category.borderStyle[k].flagDefined) {
						if (category.borderWidth[k].flagDefined && Math::isAlmostZero(category.borderWidth[k].amount)) {
							params->sbDefineInit->add(String::format("%s%s->setBorder(slib::Ref<slib::Pen>::null(), slib::ButtonState::%s, %d, slib::UIUpdateMode::Init);%n", strTab, name, strStates[k], i));
						} else {
							StringBuffer* sb;
							String strUpdateMode;
							if (category.borderWidth[k].isNeededOnLayoutFunction()) {
								sb = params->sbDefineLayout;
								strUpdateMode = "slib::UIUpdateMode::NoRedraw";
							} else {
								sb = params->sbDefineInit;
								strUpdateMode = "slib::UIUpdateMode::Init";
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
							sb->add(String::format("%s%s->setBorder(slib::Pen::create(%s, %s, %s), slib::ButtonState::%s, %d, %s);%n", strTab, name, _borderStyle, _borderWidth, _borderColor, strStates[k], i, strUpdateMode));
						}
					}
				} else if (op == OP_SIMULATE) {
					if (category.textColor[k].flagDefined) {
						if (!flagOnLayout) {
							view->setTextColor(category.textColor[k].value, states[k], i, UIUpdateMode::Init);
						}
					}
					if (category.background[k].flagDefined) {
						if (!(_checkDrawableValueAvailable(category.background[k], resourceItem->element))) {
							return sl_false;
						}
						if (flagOnLayout) {
							view->setBackground(_getDrawableValue(category.background[k]), states[k], i, UIUpdateMode::Redraw);
						}
					}
					if (category.icon[k].flagDefined) {
						if (!(_checkDrawableValueAvailable(category.icon[k], resourceItem->element))) {
							return sl_false;
						}
						if (flagOnLayout) {
							view->setIcon(_getDrawableValue(category.icon[k]), states[k], i, UIUpdateMode::Redraw);
						}
					}
					if (category.borderWidth[k].flagDefined || category.borderColor[k].flagDefined || category.borderStyle[k].flagDefined) {
						if (category.borderWidth[k].flagDefined && Math::isAlmostZero(category.borderWidth[k].amount)) {
							if (!flagOnLayout) {
								view->setBorder(Ref<Pen>::null(), states[k], i, UIUpdateMode::Init);
							}
						} else {
							if (flagOnLayout) {
								sl_real _borderWidth;
								Color _borderColor;
								PenStyle _borderStyle;
								if (category.borderWidth[k].flagDefined) {
									_borderWidth = _getDimensionFloatValue(category.borderWidth[k]);
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
								view->setBorder(Pen::create(_borderStyle, _borderWidth, _borderColor), states[k], i, UIUpdateMode::NoRedraw);
							}
						}
					}
				}
			}
		}
		
		LAYOUT_CONTROL_SET_NATIVE_WIDGET
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Label, LabelView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_STRING_ATTR_NOREDRAW(text, setText)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textColor, setTextColor)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(gravity, setGravity)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(multiLine, setMultiLineMode)

		LAYOUT_CONTROL_SET_NATIVE_WIDGET

		LAYOUT_CONTROL_ADD_STATEMENT

	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Check, CheckBox)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(Button)
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(checked, setChecked)
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Radio, RadioButton)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(Check)
		
		if (op == OP_PARSE) {
			attr->group = resourceItem->getXmlAttribute("group");
			if (attr->group.isNotEmpty()) {
				if (!(SDevUtil::checkName(attr->group.getData(), attr->group.getLength()))) {
					_logError(element, _g_sdev_sapp_error_resource_layout_name_invalid.arg(attr->group));
					return sl_false;
				}
				params->resource->radioGroups.put(attr->group, sl_true);
			}
		} else if (op == OP_GENERATE_CPP) {
			if (attr->group.isNotEmpty()) {
				params->sbDefineInit->add(String::format("%s" RADIOGROUP_NAME_PREFIX "%s->add(%s);%n", strTab, attr->group, name));
			}
		} else if (op == OP_SIMULATE) {
			if (!flagOnLayout) {
				if (attr->group.isNotEmpty()) {
					Ref<RadioGroup> group = params->simulator->getRadioGroup(attr->group);
					if (group.isNotNull()) {
						group->add(view);
					}
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Edit, EditView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_STRING_ATTR_NOREDRAW(text, setText)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(gravity, setGravity)
		LAYOUT_CONTROL_STRING_ATTR_NOREDRAW(hintText, setHintText)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(readOnly, setReadOnly)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(multiLine, setMultiLine)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textColor, setTextColor)
		LAYOUT_CONTROL_GENERIC_ATTR(returnKey, setReturnKeyType)
		LAYOUT_CONTROL_GENERIC_ATTR(keyboard, setKeyboardType)
		LAYOUT_CONTROL_GENERIC_ATTR(autoCap, setAutoCapitalizationType)
		
		LAYOUT_CONTROL_SET_NATIVE_WIDGET
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


#define SAppLayoutPasswordAttributes SAppLayoutEditAttributes
#define attrsPassword attrsEdit
	BEGIN_PROCESS_LAYOUT_CONTROL(Password, PasswordView)
	{
		if (!(_processLayoutResourceControl_Edit(params))) {
			return sl_false;
		}
	}
	END_PROCESS_LAYOUT_CONTROL


#define SAppLayoutTextAreaAttributes SAppLayoutEditAttributes
#define attrsTextArea attrsEdit
	BEGIN_PROCESS_LAYOUT_CONTROL(TextArea, TextArea)
	{
		if (!(_processLayoutResourceControl_Edit(params))) {
			return sl_false;
		}
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Image, ImageView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_DRAWABLE_ATTR(src, setSource)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(gravity, setGravity)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(scale, setScaleMode)
		
		if (!(resourceItem->attrsView->aspectRatio.flagDefined)) {
			if (op == OP_GENERATE_CPP) {
				if (resourceItem->attrsView->width.flagDefined) {
					if (!(resourceItem->attrsView->height.flagDefined)) {
						params->sbDefineInit->add(String::format("%s%s->setAspectRatioMode(slib::AspectRatioMode::AdjustHeight, slib::UIUpdateMode::Init);%n", strTab, name));
					}
				} else {
					if (resourceItem->attrsView->height.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setAspectRatioMode(slib::AspectRatioMode::AdjustWidth, slib::UIUpdateMode::Init);%n", strTab, name));
					}
				}
			} else if (op == OP_SIMULATE) {
				if (!flagOnLayout) {
					if (resourceItem->attrsView->width.flagDefined) {
						if (!(resourceItem->attrsView->height.flagDefined)) {
							view->setAspectRatioMode(AspectRatioMode::AdjustHeight, slib::UIUpdateMode::Init);
						}
					} else {
						if (resourceItem->attrsView->height.flagDefined) {
							view->setAspectRatioMode(AspectRatioMode::AdjustWidth, slib::UIUpdateMode::Init);
						}
					}
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Select, SelectView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_DRAWABLE_ATTR(leftIcon, setLeftIcon)
		LAYOUT_CONTROL_DRAWABLE_ATTR(rightIcon, setRightIcon)
		
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconWidth, setIconWidth, checkScalarSize)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(iconHeight, setIconHeight, checkScalarSize)
		if (op == OP_PARSE) {
			SAppDimensionValue iconSize;
			LAYOUT_CONTROL_PARSE_ATTR(, iconSize)
			if (!(iconSize.checkScalarSize())) {
				LOG_ERROR_LAYOUT_CONTROL_ATTR(iconSize)
				return sl_false;
			}
			if (iconSize.flagDefined) {
				if (!(attr->iconWidth.flagDefined)) {
					attr->iconWidth = iconSize;
				}
				if (!(attr->iconHeight.flagDefined)) {
					attr->iconHeight = iconSize;
				}
			}
		}
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textColor, setTextColor)
		
		LAYOUT_CONTROL_SET_NATIVE_WIDGET
		
		if (op == OP_PARSE) {
			ListElements< Ref<XmlElement> > itemXmls(_getLayoutItemChildElements(resourceItem, "item"));
			for (sl_size i = 0; i < itemXmls.count; i++) {
				Ref<XmlElement>& itemXml = itemXmls[i];
				if (itemXml.isNotNull()) {
					SAppLayoutSelectItem subItem;
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., title)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., value)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., selected)
					subItem.element = itemXml;
					if (!(attr->items.add(subItem))) {
						_logError(itemXml, _g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutSelectItem> selectItems(attr->items);
			if (selectItems.count > 0) {
				sl_size indexSelected = 0;
				sl_bool flagSelected = sl_false;
				params->sbDefineInit->add(String::format("%s%s->setItemsCount(%d, slib::UIUpdateMode::Init);%n", strTab, name, selectItems.count));
				for (sl_size i = 0; i < selectItems.count; i++) {
					SAppLayoutSelectItem& selectItem = selectItems[i];
					if (!(_checkStringValueAvailable(selectItem.title, selectItem.element))) {
						return sl_false;
					}
					if (selectItem.title.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemTitle(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, selectItem.title.getAccessString()));
					}
					if (!(_checkStringValueAvailable(selectItem.value, selectItem.element))) {
						return sl_false;
					}
					if (selectItem.value.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemValue(%d, %s);%n", strTab, name, i, selectItem.value.getAccessString()));
					}
					if (selectItem.selected.flagDefined && selectItem.selected.value) {
						flagSelected = sl_true;
						indexSelected = i;
					}
				}
				if (flagSelected) {
					params->sbDefineInit->add(String::format("%s%s->selectIndex(%d, slib::UIUpdateMode::Init);%n", strTab, name, indexSelected));
				}
			}
		} else if (op == OP_SIMULATE) {
			if (!flagOnLayout) {
				ListLocker<SAppLayoutSelectItem> selectItems(attr->items);
				if (selectItems.count > 0) {
					sl_uint32 indexSelected = 0;
					sl_bool flagSelected = sl_false;
					sl_uint32 n = (sl_uint32)(selectItems.count);
					view->setItemsCount(n, UIUpdateMode::Init);
					for (sl_uint32 i = 0; i < n; i++) {
						SAppLayoutSelectItem& selectItem = selectItems[i];
						if (!(_checkStringValueAvailable(selectItem.title, selectItem.element))) {
							return sl_false;
						}
						if (selectItem.title.flagDefined) {
							view->setItemTitle(i, _getStringValue(selectItem.title), UIUpdateMode::Init);
						}
						if (!(_checkStringValueAvailable(selectItem.value, selectItem.element))) {
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
						if (!flagOnLayout) {
							view->selectIndex(indexSelected, UIUpdateMode::Init);
						}
					}
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Scroll, ScrollView)
	{

		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
		if (op == OP_PARSE) {
			sl_size nChildren = element->getChildElementsCount();
			if (nChildren > 0) {
				if (nChildren != 1) {
					_logError(element, _g_sdev_sapp_error_resource_layout_scrollview_must_contain_one_child);
					return sl_false;
				}
				Ref<XmlElement> xmlChild = element->getFirstChildElement();
				if (xmlChild.isNotNull()) {
					Ref<SAppLayoutResourceItem> contentItem = _parseLayoutResourceItemChild(params->resource, resourceItem, xmlChild);
					if (contentItem.isNull()) {
						return sl_false;
					}
					attr->content = contentItem;
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			if (attr->content.isNotNull()) {
				String addChildStatement = String::format("%s%s->setContentView(%s, slib::UIUpdateMode::Init);%n%n", strTab, name, attr->content->name);
				if (!(_generateLayoutsCppItem(params->resource, attr->content.get(), resourceItem, *(params->sbDeclare), *(params->sbDefineInit), *(params->sbDefineLayout), addChildStatement))) {
					return sl_false;
				}
			}
		} else if (op == OP_SIMULATE) {
			if (attr->content.isNotNull()) {
				Ref<View> contentView = _simulateLayoutCreateOrLayoutView(params->simulator, attr->content.get(), resourceItem, view, flagOnLayout);
				if (contentView.isNotNull()) {
					if (!flagOnLayout) {
						view->setContentView(contentView, UIUpdateMode::Init);
					}
				} else {
					return sl_false;
				}
			}
		}
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Linear, LinearView)
	{
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(orientation, setOrientation)
		if (!(attr->orientation.flagDefined)) {
			if (op == OP_PARSE) {
				if (resourceItem->element->getName() == "hlinear") {
					attr->orientation.flagDefined = sl_true;
					attr->orientation.value = LayoutOrientation::Horizontal;
				}
			}
		}
		
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	class _SAppDocument_SimuationListViewAdapter : public Referable, public IListViewAdapter
	{
	public:
		WeakRef<Referable> refer;
		SAppLayoutSimulator* simulator;
		Ref<SAppLayoutResource> layout;
		
	public:
		sl_uint64 getItemsCount(ListView* lv)
		{
			return 100;
		}
		
		Ref<View> getView(ListView* lv, sl_uint64 index, View* original)
		{
			if (original) {
				return original;
			}
			Ref<Referable> _refer = refer;
			if (_refer.isNull()) {
				return Ref<View>::null();
			}
			Ref<SAppLayoutImportView> view = new SAppLayoutImportView;
			if (view.isNotNull()) {
				view->init(simulator, layout.get());
			}
			return view;
		}
		
	};

	BEGIN_PROCESS_LAYOUT_CONTROL(List, ListView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		if (op == OP_PARSE) {
			attr->itemLayout = resourceItem->getXmlAttribute("item");
		} else if (op == OP_SIMULATE) {
			if (attr->itemLayout.isNotEmpty()) {
				
				Ref<SAppLayoutResource> layoutItem;
				m_layouts.get(attr->itemLayout, &layoutItem);
				if (layoutItem.isNull()) {
					_logError(element, _g_sdev_sapp_error_layout_not_found.arg(attr->itemLayout));
					return sl_false;
				}
				if (layoutItem->layoutType != SAppLayoutType::View) {
					_logError(element, _g_sdev_sapp_error_layout_is_not_view.arg(attr->itemLayout));
					return sl_false;
				}
				
				Ref<_SAppDocument_SimuationListViewAdapter> adapter = new _SAppDocument_SimuationListViewAdapter;
				adapter->refer = params->simulator->getReferable();
				adapter->simulator = params->simulator;
				adapter->layout = layoutItem;
				
				view->setAdapter(adapter);
				
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT

	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(ListReport, ListReportView)
	{
		
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		if (op == OP_PARSE) {
			ListLocker< Ref<XmlElement> > columnXmls(_getLayoutItemChildElements(resourceItem, "column"));
			for (sl_size i = 0; i < columnXmls.count; i++) {
				Ref<XmlElement>& columnXml = columnXmls[i];
				if (columnXml.isNotNull()) {
					SAppLayoutListReportColumn column;
					LAYOUT_CONTROL_PARSE_XML_ATTR(columnXml, column., title)
					LAYOUT_CONTROL_PARSE_XML_ATTR(columnXml, column., width)
					if (!(column.width.checkScalarSize())) {
						LOG_ERROR_LAYOUT_CONTROL_XML_ATTR(columnXml, width)
						return sl_false;
					}
					LAYOUT_CONTROL_PARSE_XML_ATTR(columnXml, column., align)
					LAYOUT_CONTROL_PARSE_XML_ATTR(columnXml, column., headerAlign)
					
					if (!(attr->columns.add(column))) {
						_logError(columnXml, _g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutListReportColumn> columns(attr->columns);
			if (columns.count > 0) {
				params->sbDefineInit->add(String::format("%s%s->setColumnsCount(%d, slib::UIUpdateMode::Init);%n", strTab, name, columns.count));
				for (sl_size i = 0; i < columns.count; i++) {
					SAppLayoutListReportColumn& column = columns[i];
					if (!(_checkStringValueAvailable(column.title, element))) {
						return sl_false;
					}
					if (column.title.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setHeaderText(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, column.title.getAccessString()));
					}
					if (column.width.flagDefined) {
						if (column.width.isNeededOnLayoutFunction()) {
							params->sbDefineLayout->add(String::format("%s%s->setColumnWidth(%d, %s, slib::UIUpdateMode::NoRedraw);%n", strTab, name, i, column.width.getAccessString()));
						} else {
							params->sbDefineInit->add(String::format("%s%s->setColumnWidth(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, column.width.getAccessString()));
						}
					}
					if (column.align.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setColumnAlignment(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, column.align.getAccessString()));
					}
					if (column.headerAlign.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setHeaderAlignment(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, column.headerAlign.getAccessString()));
					}
				}
			}
		} else if (op == OP_SIMULATE) {
			ListLocker<SAppLayoutListReportColumn> columns(attr->columns);
			if (columns.count > 0) {
				sl_uint32 n = (sl_uint32)(columns.count);
				if (!flagOnLayout) {
					view->setColumnsCount(n, UIUpdateMode::Init);
				}
				for (sl_uint32 i = 0; i < n; i++) {
					SAppLayoutListReportColumn& column = columns[i];
					if (!flagOnLayout) {
						if (!(_checkStringValueAvailable(column.title, element))) {
							return sl_false;
						}
						if (column.title.flagDefined) {
							view->setHeaderText(i, _getStringValue(column.title), UIUpdateMode::Init);
						}
						if (column.align.flagDefined) {
							view->setColumnAlignment(i, column.align.value, UIUpdateMode::Init);
						}
						if (column.headerAlign.flagDefined) {
							view->setHeaderAlignment(i, column.headerAlign.value, UIUpdateMode::Init);
						}
					}
					if (column.width.flagDefined) {
						if (flagOnLayout) {
							view->setColumnWidth(i, _getDimensionIntValue(column.width), UIUpdateMode::NoRedraw);
						}
					}
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Render, RenderView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_GENERIC_ATTR(redraw, setRedrawMode)

		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Tab, TabView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(orientation, setOrientation)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(tabWidth, setTabWidth, checkScalarSize)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(tabHeight, setTabHeight, checkScalarSize)
		LAYOUT_CONTROL_DRAWABLE_ATTR(barBackground, setBarBackground)
		LAYOUT_CONTROL_DRAWABLE_ATTR(contentBackground, setContentBackground)
		LAYOUT_CONTROL_DRAWABLE_ATTR(tabBackground, setTabBackground)
		LAYOUT_CONTROL_DRAWABLE_ATTR(selectedTabBackground, setSelectedTabBackground)
		LAYOUT_CONTROL_DRAWABLE_ATTR(hoverTabBackground, setHoverTabBackground)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(labelColor, setLabelColor)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(selectedLabelColor, setSelectedLabelColor)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(hoverLabelColor, setHoverLabelColor)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(labelAlign, setLabelAlignment)
		
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(labelMarginLeft, setLabelMarginLeft, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(labelMarginTop, setLabelMarginTop, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(labelMarginRight, setLabelMarginRight, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(labelMarginBottom, setLabelMarginBottom, checkPosition)
		if (op == OP_PARSE) {
			SAppDimensionValue labelMargin;
			LAYOUT_CONTROL_PARSE_ATTR(, labelMargin)
			if (!(labelMargin.checkPosition())) {
				LOG_ERROR_LAYOUT_CONTROL_ATTR(labelMargin)
				return sl_false;
			}
			if (labelMargin.flagDefined) {
				if (!(attr->labelMarginLeft.flagDefined)) {
					attr->labelMarginLeft = labelMargin;
				}
				if (!(attr->labelMarginTop.flagDefined)) {
					attr->labelMarginTop = labelMargin;
				}
				if (!(attr->labelMarginRight.flagDefined)) {
					attr->labelMarginRight = labelMargin;
				}
				if (!(attr->labelMarginBottom.flagDefined)) {
					attr->labelMarginBottom = labelMargin;
				}
			}
		}

		if (op == OP_PARSE) {
			ListLocker< Ref<XmlElement> > itemXmls(_getLayoutItemChildElements(resourceItem, "item"));
			for (sl_size i = 0; i < itemXmls.count; i++) {
				Ref<XmlElement>& itemXml = itemXmls[i];
				if (itemXml.isNotNull()) {
					SAppLayoutTabItem subItem;
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., label)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., selected)
					
					sl_size nViews = itemXml->getChildElementsCount();
					if (nViews > 0) {
						if (nViews != 1) {
							_logError(itemXml, _g_sdev_sapp_error_resource_layout_item_must_contain_one_child);
							return sl_false;
						}
						Ref<XmlElement> xmlView= itemXml->getFirstChildElement();
						if (xmlView.isNotNull()) {
							Ref<SAppLayoutResourceItem> subItemView = _parseLayoutResourceItemChild(params->resource, resourceItem, xmlView);
							if (subItemView.isNull()) {
								return sl_false;
							}
							subItem.view = subItemView;
						}
					}
					
					subItem.element = itemXml;
					
					if (!(attr->items.add(subItem))) {
						_logError(itemXml, _g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutTabItem> subItems(attr->items);
			if (subItems.count > 0) {
				sl_size indexSelected = 0;
				sl_bool flagSelected = sl_false;
				
				params->sbDefineInit->add(String::format("%s%s->setTabsCount(%d, slib::UIUpdateMode::Init);%n", strTab, name, subItems.count));
				
				for (sl_size i = 0; i < subItems.count; i++) {
					SAppLayoutTabItem& subItem = subItems[i];
					if (subItem.label.flagDefined) {
						if (!(_checkStringValueAvailable(subItem.label, subItem.element))) {
							return sl_false;
						}
						params->sbDefineInit->add(String::format("%s%s->setTabLabel(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.label.getAccessString()));
					}
					if (subItem.selected.flagDefined && subItem.selected.value) {
						flagSelected = sl_true;
						indexSelected = i;
					}
				}
				
				if (flagSelected) {
					params->sbDefineInit->add(String::format("%s%s->selectTab(%d, slib::UIUpdateMode::Init);%n", strTab, name, indexSelected));
				}
			}
		} else if (op == OP_SIMULATE) {
			ListLocker<SAppLayoutTabItem> subItems(attr->items);
			if (subItems.count > 0) {
				
				sl_uint32 indexSelected = 0;
				sl_bool flagSelected = sl_false;
				
				sl_uint32 nSubItems = (sl_uint32)(subItems.count);
				if (!flagOnLayout) {
					view->setTabsCount(nSubItems, UIUpdateMode::Init);
				}
				
				for (sl_uint32 i = 0; i < nSubItems; i++) {
					
					SAppLayoutTabItem& subItem = subItems[i];
					if (subItem.label.flagDefined) {
						if (!flagOnLayout) {
							if (!(_checkStringValueAvailable(subItem.label, subItem.element))) {
								return sl_false;
							}
							view->setTabLabel(i, _getStringValue(subItem.label), UIUpdateMode::Init);
						}
					}
					if (subItem.selected.flagDefined && subItem.selected.value) {
						flagSelected = sl_true;
						indexSelected = i;
					}
					
					if (subItem.view.isNotNull()) {
						Ref<View> contentView = _simulateLayoutCreateOrLayoutView(params->simulator, subItem.view.get(), resourceItem, view, flagOnLayout);
						if (contentView.isNotNull()) {
							if (!flagOnLayout) {
								view->setTabContentView(i, contentView, UIUpdateMode::Init);
							}
						} else {
							return sl_false;
						}
					}
					
					if (flagSelected) {
						if (!flagOnLayout) {
							view->selectTab(indexSelected, UIUpdateMode::Init);
						}
					}
					
				}
				
			}

		}
		
		LAYOUT_CONTROL_SET_NATIVE_WIDGET

		LAYOUT_CONTROL_ADD_STATEMENT
		
		if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutTabItem> subItems(attr->items);
			for (sl_size i = 0; i < subItems.count; i++) {
				SAppLayoutTabItem& subItem = subItems[i];
				if (subItem.view.isNotNull()) {
					String addChildStatement = String::format("%s%s->setTabContentView(%d, %s, slib::UIUpdateMode::Init);%n%n", strTab, name, i, subItem.view->name);
					if (!(_generateLayoutsCppItem(params->resource, subItem.view.get(), resourceItem, *(params->sbDeclare), *(params->sbDefineInit), *(params->sbDefineLayout), addChildStatement))) {
						return sl_false;
					}
				}
			}
		}
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Tree, TreeView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_DRAWABLE_ATTR(itemIcon, setItemIcon)
		LAYOUT_CONTROL_DRAWABLE_ATTR(opendIcon, setOpenedItemIcon)
		LAYOUT_CONTROL_DRAWABLE_ATTR(closedIcon, setClosedItemIcon)
		LAYOUT_CONTROL_DRAWABLE_ATTR(collapsedIcon, setCollapsedIcon)
		LAYOUT_CONTROL_DRAWABLE_ATTR(expandedIcon, setExpandedIcon)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(selectedBackgroundColor, setSelectedItemBackgroundColor)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textColor, setItemTextColor)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(hoverTextColor, setHoverItemTextColor)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(selectedTextColor, setSelectedItemTextColor)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(itemHeight, setItemHeight, checkScalarSize)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(itemPadding, setItemPadding, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(itemIndent, setItemIndent, checkPosition)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(textIndent, setTextIndent, checkPosition)
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Split, SplitView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(orientation, setOrientation)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(dividerWidth, setDividerWidth, checkScalarSize)
		LAYOUT_CONTROL_DRAWABLE_ATTR(dividerBackground, setDividerBackground)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(dividerColor, setDividerColor)
		LAYOUT_CONTROL_INT_DIMENSION_ATTR(cursorMargin, setCursorMargin, checkScalarSize)
		
		sl_bool flagRelayoutOnInit = sl_false;
		sl_bool flagRelayoutOnLayout = sl_false;
		
		if (op == OP_PARSE) {
			ListLocker< Ref<XmlElement> > itemXmls(_getLayoutItemChildElements(resourceItem, "item"));
			for (sl_size i = 0; i < itemXmls.count; i++) {
				Ref<XmlElement>& itemXml = itemXmls[i];
				if (itemXml.isNotNull()) {
					SAppLayoutSplitItem subItem;
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., weight)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., minWeight)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., maxWeight)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., minSize)
					if (!(subItem.minSize.checkScalarSize())) {
						LOG_ERROR_LAYOUT_CONTROL_XML_ATTR(itemXml, minSize)
						return sl_false;
					}
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., maxSize)
					if (!(subItem.maxSize.checkScalarSize())) {
						LOG_ERROR_LAYOUT_CONTROL_XML_ATTR(itemXml, maxSize)
						return sl_false;
					}
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., dividerWidth)
					if (!(subItem.dividerWidth.checkScalarSize())) {
						LOG_ERROR_LAYOUT_CONTROL_XML_ATTR(itemXml, dividerWidth)
						return sl_false;
					}
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., dividerBackground)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., dividerColor)
					
					sl_size nViews = itemXml->getChildElementsCount();
					if (nViews > 0) {
						if (nViews != 1) {
							_logError(itemXml, _g_sdev_sapp_error_resource_layout_item_must_contain_one_child);
							return sl_false;
						}
						Ref<XmlElement> xmlView= itemXml->getFirstChildElement();
						if (xmlView.isNotNull()) {
							Ref<SAppLayoutResourceItem> subItemView = _parseLayoutResourceItemChild(params->resource, resourceItem, xmlView);
							if (subItemView.isNull()) {
								return sl_false;
							}
							subItem.view = subItemView;
						}
					}
					
					subItem.element = itemXml;
					
					if (!(attr->items.add(subItem))) {
						_logError(itemXml, _g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutSplitItem> subItems(attr->items);
			if (subItems.count > 0) {
				if (subItems.count > 2) {
					params->sbDefineInit->add(String::format("%s%s->setItemsCount(%d, slib::UIUpdateMode::Init);%n", strTab, name, subItems.count));
				}
				
				for (sl_size i = 0; i < subItems.count; i++) {
					SAppLayoutSplitItem& subItem = subItems[i];
					if (subItem.weight.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemWeight(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.weight.getAccessString()));
						flagRelayoutOnInit = sl_true;
					}
					if (subItem.minWeight.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemMinimumWeight(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.minWeight.getAccessString()));
						flagRelayoutOnInit = sl_true;
					}
					if (subItem.maxWeight.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemMaximumWeight(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.maxWeight.getAccessString()));
						flagRelayoutOnInit = sl_true;
					}
					if (subItem.minSize.flagDefined) {
						if (subItem.minSize.isNeededOnLayoutFunction()) {
							params->sbDefineLayout->add(String::format("%s%s->setItemMinimumSize(%d, %s, slib::UIUpdateMode::NoRedraw);%n", strTab, name, i, subItem.minSize.getAccessString()));
							flagRelayoutOnLayout = sl_true;
						} else {
							params->sbDefineInit->add(String::format("%s%s->setItemMinimumSize(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.minSize.getAccessString()));
							flagRelayoutOnInit = sl_true;
						}
					}
					if (subItem.maxSize.flagDefined) {
						if (subItem.maxSize.isNeededOnLayoutFunction()) {
							params->sbDefineLayout->add(String::format("%s%s->setItemMaximumSize(%d, %s, slib::UIUpdateMode::NoRedraw);%n", strTab, name, i, subItem.maxSize.getAccessString()));
							flagRelayoutOnLayout = sl_true;
						} else {
							params->sbDefineInit->add(String::format("%s%s->setItemMaximumSize(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.maxSize.getAccessString()));
							flagRelayoutOnInit = sl_true;
						}
					}
					if (subItem.dividerWidth.flagDefined) {
						if (subItem.dividerWidth.isNeededOnLayoutFunction()) {
							params->sbDefineLayout->add(String::format("%s%s->setItemDividerWidth(%d, %s, slib::UIUpdateMode::NoRedraw);%n", strTab, name, i, subItem.dividerWidth.getAccessString()));
							flagRelayoutOnLayout = sl_true;
						} else {
							params->sbDefineInit->add(String::format("%s%s->setItemDividerWidth(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.dividerWidth.getAccessString()));
							flagRelayoutOnInit = sl_true;
						}
					}
					if (subItem.dividerBackground.flagDefined) {
						if (!(_checkDrawableValueAvailable(subItem.dividerBackground, subItem.element))) {
							return sl_false;
						}
						params->sbDefineInit->add(String::format("%s%s->setItemDividerBackground(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.dividerBackground.getAccessString()));
						flagRelayoutOnInit = sl_true;
					}
					if (subItem.dividerColor.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemDividerColor(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, subItem.dividerColor.getAccessString()));
						flagRelayoutOnInit = sl_true;
					}
				}
			}
		} else if (op == OP_SIMULATE) {
			
			ListLocker<SAppLayoutSplitItem> subItems(attr->items);
			if (subItems.count > 0) {
				if (!flagOnLayout) {
					if (subItems.count > 2) {
						view->setItemsCount(subItems.count, UIUpdateMode::Init);
					}
				}
				for (sl_size i = 0; i < subItems.count; i++) {
					SAppLayoutSplitItem& subItem = subItems[i];
					if (subItem.weight.flagDefined) {
						if (!flagOnLayout) {
							view->setItemWeight(i, subItem.weight.value, UIUpdateMode::Init);
						}
					}
					if (subItem.minWeight.flagDefined) {
						if (!flagOnLayout) {
							view->setItemMinimumWeight(i, subItem.minWeight.value, UIUpdateMode::Init);
						}
					}
					if (subItem.maxWeight.flagDefined) {
						if (!flagOnLayout) {
							view->setItemMaximumWeight(i, subItem.maxWeight.value, UIUpdateMode::Init);
						}
					}
					if (subItem.minSize.flagDefined) {
						if (flagOnLayout) {
							view->setItemMinimumSize(i, _getDimensionIntValue(subItem.minSize), UIUpdateMode::NoRedraw);
						}
					}
					if (subItem.maxSize.flagDefined) {
						if (flagOnLayout) {
							view->setItemMaximumSize(i, _getDimensionIntValue(subItem.maxSize), UIUpdateMode::NoRedraw);
						}
					}
					if (subItem.dividerWidth.flagDefined) {
						if (flagOnLayout) {
							view->setItemDividerWidth(i, _getDimensionIntValue(subItem.dividerWidth), UIUpdateMode::NoRedraw);
						}
					}
					if (subItem.dividerBackground.flagDefined) {
						if (!(_checkDrawableValueAvailable(subItem.dividerBackground, subItem.element))) {
							return sl_false;
						}
						if (flagOnLayout) {
							view->setItemDividerBackground(i, _getDrawableValue(subItem.dividerBackground), UIUpdateMode::NoRedraw);
						}
					}
					if (subItem.dividerColor.flagDefined) {
						if (!flagOnLayout) {
							view->setItemDividerColor(i, subItem.dividerColor.value, UIUpdateMode::Init);
						}
					}
					if (subItem.view.isNotNull()) {
						Ref<View> contentView = _simulateLayoutCreateOrLayoutView(params->simulator, subItem.view.get(), resourceItem, view, flagOnLayout);
						if (contentView.isNotNull()) {
							if (!flagOnLayout) {
								view->setItemView(i, contentView, UIUpdateMode::NoRedraw);
							}
						} else {
							return sl_false;
						}
					}
				}
				
				view->relayout(UIUpdateMode::NoRedraw);
				
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
		if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutSplitItem> subItems(attr->items);
			for (sl_size i = 0; i < subItems.count; i++) {
				SAppLayoutSplitItem& subItem = subItems[i];
				if (subItem.view.isNotNull()) {
					String addChildStatement = String::format("%s%s->setItemView(%d, %s, slib::UIUpdateMode::Init);%n%n", strTab, name, i, subItem.view->name);
					if (!(_generateLayoutsCppItem(params->resource, subItem.view.get(), resourceItem, *(params->sbDeclare), *(params->sbDefineInit), *(params->sbDefineLayout), addChildStatement))) {
						return sl_false;
					}
				}
			}
			if (flagRelayoutOnInit) {
				params->sbDefineInit->add(String::format("%s%s->relayout(slib::UIUpdateMode::NoRedraw);%n%n", strTab, name));
			}
			if (flagRelayoutOnLayout) {
				params->sbDefineLayout->add(String::format("%s%s->relayout(slib::UIUpdateMode::NoRedraw);%n%n", strTab, name));
			}
		}
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Web, WebView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		if (op == OP_PARSE) {
			LAYOUT_CONTROL_PARSE_ATTR(attr->, url)
			LAYOUT_CONTROL_PARSE_ATTR(attr->, html)
		} else {
			if (!(_checkStringValueAvailable(attr->url, element))) {
				return sl_false;
			}
			if (!(_checkStringValueAvailable(attr->html, element))) {
				return sl_false;
			}
			if (op == OP_GENERATE_CPP) {
				if (attr->html.flagDefined) {
					if (attr->url.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->loadHTML(%s, %s);%n%n", strTab, name, attr->html.getAccessString(), attr->url.getAccessString()));
					} else {
						params->sbDefineInit->add(String::format("%s%s->loadHTML(%s, slib::String::null());%n%n", strTab, name, attr->html.getAccessString()));
					}
				} else {
					if (attr->url.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->loadURL(%s);%n%n", strTab, name, attr->url.getAccessString()));
					}
				}
			} else if (op == OP_SIMULATE) {
				if (!flagOnLayout) {
					if (attr->html.flagDefined) {
						if (attr->url.flagDefined) {
							view->loadHTML(_getStringValue(attr->html), _getStringValue(attr->url));
						} else {
							view->loadHTML(_getStringValue(attr->html), String::null());
						}
					} else {
						if (attr->url.flagDefined) {
							view->loadURL(_getStringValue(attr->url));
						}
					}
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Progress, ProgressBar)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(orientation, setOrientation)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(min, setMinimumValue)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(max, setMaximumValue)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(range, setRange)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(value, setValue)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(value2, setSecondaryValue)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(dual, setDualValues)
		LAYOUT_CONTROL_GENERIC_ATTR(discrete, setDiscrete)
		LAYOUT_CONTROL_GENERIC_ATTR(step, setStep)
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(reversed, setReversed)
		LAYOUT_CONTROL_DRAWABLE_ATTR(track, setTrackDrawable)
		LAYOUT_CONTROL_DRAWABLE_ATTR(progress, setProgressDrawable)
		LAYOUT_CONTROL_DRAWABLE_ATTR(progress2, setSecondaryProgressDrawable)
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Slider, Slider)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(Progress)
		
		LAYOUT_CONTROL_DRAWABLE_ATTR(thumb, setThumbDrawable)
		LAYOUT_CONTROL_DRAWABLE_ATTR(pressedThumb, setPressedThumbDrawable)
		LAYOUT_CONTROL_DRAWABLE_ATTR(hoverThumb, setHoverThumbDrawable);

		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(thumbWidth, setThumbWidth, checkScalarSize);
		LAYOUT_CONTROL_INT_DIMENSION_ATTR_NOREDRAW(thumbHeight, setThumbHeight, checkScalarSize);
		if (op == OP_PARSE) {
			SAppDimensionValue thumbSize;
			LAYOUT_CONTROL_PARSE_ATTR(, thumbSize)
			if (!(thumbSize.checkScalarSize())) {
				LOG_ERROR_LAYOUT_CONTROL_ATTR(thumbSize)
				return sl_false;
			}
			if (thumbSize.flagDefined) {
				if (!(attr->thumbWidth.flagDefined)) {
					attr->thumbWidth = thumbSize;
				}
				if (!(attr->thumbHeight.flagDefined)) {
					attr->thumbHeight = thumbSize;
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL


	BEGIN_PROCESS_LAYOUT_CONTROL(Picker, PickerView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_GENERIC_ATTR_NOREDRAW(textColor, setTextColor)
		
		LAYOUT_CONTROL_SET_NATIVE_WIDGET
		
		if (op == OP_PARSE) {
			ListElements< Ref<XmlElement> > itemXmls(_getLayoutItemChildElements(resourceItem, "item"));
			for (sl_size i = 0; i < itemXmls.count; i++) {
				Ref<XmlElement>& itemXml = itemXmls[i];
				if (itemXml.isNotNull()) {
					SAppLayoutSelectItem subItem;
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., title)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., value)
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., selected)
					subItem.element = itemXml;
					if (!(attr->items.add(subItem))) {
						_logError(itemXml, _g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutSelectItem> selectItems(attr->items);
			if (selectItems.count > 0) {
				sl_size indexSelected = 0;
				sl_bool flagSelected = sl_false;
				params->sbDefineInit->add(String::format("%s%s->setItemsCount(%d, slib::UIUpdateMode::Init);%n", strTab, name, selectItems.count));
				for (sl_size i = 0; i < selectItems.count; i++) {
					SAppLayoutSelectItem& selectItem = selectItems[i];
					if (!(_checkStringValueAvailable(selectItem.title, selectItem.element))) {
						return sl_false;
					}
					if (selectItem.title.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemTitle(%d, %s, slib::UIUpdateMode::Init);%n", strTab, name, i, selectItem.title.getAccessString()));
					}
					if (!(_checkStringValueAvailable(selectItem.value, selectItem.element))) {
						return sl_false;
					}
					if (selectItem.value.flagDefined) {
						params->sbDefineInit->add(String::format("%s%s->setItemValue(%d, %s);%n", strTab, name, i, selectItem.value.getAccessString()));
					}
					if (selectItem.selected.flagDefined && selectItem.selected.value) {
						flagSelected = sl_true;
						indexSelected = i;
					}
				}
				if (flagSelected) {
					params->sbDefineInit->add(String::format("%s%s->selectIndex(%d, slib::UIUpdateMode::Init);%n", strTab, name, indexSelected));
				}
			}
		} else if (op == OP_SIMULATE) {
			if (!flagOnLayout) {
				ListLocker<SAppLayoutSelectItem> selectItems(attr->items);
				if (selectItems.count > 0) {
					sl_uint32 indexSelected = 0;
					sl_bool flagSelected = sl_false;
					sl_uint32 n = (sl_uint32)(selectItems.count);
					view->setItemsCount(n, UIUpdateMode::Init);
					for (sl_uint32 i = 0; i < n; i++) {
						SAppLayoutSelectItem& selectItem = selectItems[i];
						if (!(_checkStringValueAvailable(selectItem.title, selectItem.element))) {
							return sl_false;
						}
						if (selectItem.title.flagDefined) {
							view->setItemTitle(i, _getStringValue(selectItem.title), UIUpdateMode::Init);
						}
						if (!(_checkStringValueAvailable(selectItem.value, selectItem.element))) {
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
						if (!flagOnLayout) {
							view->selectIndex(indexSelected, UIUpdateMode::Init);
						}
					}
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL

	BEGIN_PROCESS_LAYOUT_CONTROL(Pager, ViewPager)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_GENERIC_ATTR(swipe, setSwipeNavigation)
		
		if (op == OP_PARSE) {
			ListLocker< Ref<XmlElement> > itemXmls(_getLayoutItemChildElements(resourceItem, "item"));
			for (sl_size i = 0; i < itemXmls.count; i++) {
				Ref<XmlElement>& itemXml = itemXmls[i];
				if (itemXml.isNotNull()) {
					SAppLayoutPagerItem subItem;
					LAYOUT_CONTROL_PARSE_XML_ATTR(itemXml, subItem., selected)
					
					sl_size nViews = itemXml->getChildElementsCount();
					if (nViews > 0) {
						if (nViews != 1) {
							_logError(itemXml, _g_sdev_sapp_error_resource_layout_item_must_contain_one_child);
							return sl_false;
						}
						Ref<XmlElement> xmlView= itemXml->getFirstChildElement();
						if (xmlView.isNotNull()) {
							Ref<SAppLayoutResourceItem> subItemView = _parseLayoutResourceItemChild(params->resource, resourceItem, xmlView);
							if (subItemView.isNull()) {
								return sl_false;
							}
							subItem.view = subItemView;
						}
					}
					
					subItem.element = itemXml;
					
					if (!(attr->items.add(subItem))) {
						_logError(itemXml, _g_sdev_sapp_error_out_of_memory);
						return sl_false;
					}
				}
			}
		} else if (op == OP_GENERATE_CPP) {
			/* see below */
		} else if (op == OP_SIMULATE) {
			ListLocker<SAppLayoutPagerItem> subItems(attr->items);
			if (subItems.count > 0) {
				
				sl_uint32 indexSelected = 0;
				
				sl_uint32 nSubItems = (sl_uint32)(subItems.count);
				
				for (sl_uint32 i = 0; i < nSubItems; i++) {
					
					SAppLayoutPagerItem& subItem = subItems[i];
					
					if (subItem.selected.flagDefined && subItem.selected.value) {
						indexSelected = i;
					}
					
					if (subItem.view.isNotNull()) {
						Ref<View> contentView = _simulateLayoutCreateOrLayoutView(params->simulator, subItem.view.get(), resourceItem, view, flagOnLayout);
						if (contentView.isNotNull()) {
							if (!flagOnLayout) {
								view->addPage(contentView, UIUpdateMode::Init);
							}
						} else {
							return sl_false;
						}
					}
					
					if (!flagOnLayout) {
						view->selectPage(indexSelected, UIUpdateMode::Init);
					}
					
				}
				
			}
			
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
		if (op == OP_GENERATE_CPP) {
			ListLocker<SAppLayoutPagerItem> subItems(attr->items);
			if (subItems.count > 0) {
				sl_size indexSelected = 0;
				for (sl_size i = 0; i < subItems.count; i++) {
					SAppLayoutPagerItem& subItem = subItems[i];
					if (subItem.view.isNotNull()) {
						String addChildStatement = String::format("%s%s->addPage(%s, slib::UIUpdateMode::Init);%n%n", strTab, name, subItem.view->name);
						if (!(_generateLayoutsCppItem(params->resource, subItem.view.get(), resourceItem, *(params->sbDeclare), *(params->sbDefineInit), *(params->sbDefineLayout), addChildStatement))) {
							return sl_false;
						}
					}
					if (subItem.selected.flagDefined && subItem.selected.value) {
						indexSelected = i;
					}
				}
				params->sbDefineInit->add(String::format("%s%s->selectPage(%d, slib::UIUpdateMode::Init);%n", strTab, name, indexSelected));
			}
		}
		
	}
	END_PROCESS_LAYOUT_CONTROL

	BEGIN_PROCESS_LAYOUT_CONTROL(Video, VideoView)
	{
		LAYOUT_CONTROL_PROCESS_SUPER(View)
		
		LAYOUT_CONTROL_GENERIC_ATTR(repeat, setRepeat)
		
		if (op == OP_PARSE || op == OP_GENERATE_CPP) {
			LAYOUT_CONTROL_STRING_ATTR(src, setSource)
		} else {
			if (!flagOnLayout) {
				if (attr->src.flagDefined) {
					if (!(_checkStringValueAvailable(attr->src, element))) {
						return sl_false;
					}
					String value = _getStringValue(attr->src);
					if (value.startsWith("asset://")) {
						value = m_pathApp + "/asset/" + value.substring(8);
					}
					view->setSource(value);
				}
			}
		}
		
		LAYOUT_CONTROL_ADD_STATEMENT
		
	}
	END_PROCESS_LAYOUT_CONTROL

}
