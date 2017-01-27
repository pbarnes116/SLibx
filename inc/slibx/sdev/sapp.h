#ifndef CHECKHEADER_SLIB_SDEV_SAPP
#define CHECKHEADER_SLIB_SDEV_SAPP

#include "definition.h"

#include "sapp_values.h"
#include "sapp_resources.h"

SLIB_SDEV_NAMESPACE_BEGIN

class SAppConfiguration
{
public:
	String generate_cpp_target_path;
	String generate_cpp_namespace;
	CList<String> generate_cpp_layout_include_headers;
	CList<String> generate_cpp_layout_include_headers_in_cpp;
	
};

class SAppDocument : public Object
{
public:
	SAppDocument();
	~SAppDocument();

public:
	sl_bool open(const String& filePath);
	
	void close();
	
	sl_bool isOpened();
	
	sl_bool openResources();
	
	sl_bool generateCpp();
	
	List< Ref<SAppLayoutResource> > getLayouts();
	
	void simulateLayoutInWindow(const String& layoutName);
	
protected:
	// Log
	void _log(const String& text);
	void _logError(const String& text);
	void _logError(const String& filePath, sl_size line, sl_size col, const String& text);
	void _logError(const Ref<XmlElement>& element, const String& text);

	// Utilities
	String _getShortcutKeyDefinitionString(const KeycodeAndModifiers& km, sl_bool flagMac);

	// Resources Entry
	sl_bool _parseConfiguration(const String& filePath, SAppConfiguration& conf);
	void _freeResources();
	sl_bool _parseResourcesXml(const String& filePath);
	sl_bool _generateResourcesH(const String& targetPath);
	
	// Raw Resources
	sl_bool _registerRawResources(const String& resourcePath, const String& fileDirPath);
	sl_bool _registerRawResource(const String& path, const String& filePath, String& outName);
	sl_bool _generateRawsCpp(const String& targetPath);

	// Drawable Resources
	sl_bool _generateDrawablesCpp(const String& targetPath);
	Ref<Drawable> _getDrawableValue(SAppDrawableValue& value);
	Ref<Image> _getImageValue(SAppDrawableValue& value);
	sl_bool _registerImageResources(const String& resourcePath, const String& fileDirPath, const Locale& locale);
	sl_bool _generateDrawablesCpp_Image(SAppDrawableResource* res, StringBuffer& sbHeader, StringBuffer& sbCpp, StringBuffer& sbMap);
	Ref<Drawable> _getDrawableValue_Image(SAppDrawableResource* res);
	sl_bool _parseNinePiecesDrawableResource(const Ref<XmlElement>& element);
	sl_bool _generateDrawablesCpp_NinePieces(SAppDrawableResource* res, StringBuffer& sbHeader, StringBuffer& sbCpp, StringBuffer& sbMap);
	Ref<Drawable> _getDrawableValue_NinePieces(SAppDrawableResource* res);
	sl_bool _parseNinePatchDrawableResource(const Ref<XmlElement>& element);
	sl_bool _generateDrawablesCpp_NinePatch(SAppDrawableResource* res, StringBuffer& sbHeader, StringBuffer& sbCpp, StringBuffer& sbMap);
	Ref<Drawable> _getDrawableValue_NinePatch(SAppDrawableResource* res);
	sl_bool _checkDrawableValueAvailable(SAppDrawableValue& value, const Ref<XmlElement>& elementError);
	sl_bool _checkDrawableValueImage(SAppDrawableValue& value, const Ref<XmlElement>& elementError);
	
	// String Resources
	sl_bool _parseStringResources(const Ref<XmlElement>& element);
	sl_bool _parseStringResource(const Ref<XmlElement>& element, Locale locale);
	sl_bool _generateStringsCpp(const String& targetPath);
	String _getStringValue(const SAppStringValue& value);
	sl_bool _checkStringValueAvailable(SAppStringValue& value, const Ref<XmlElement>& elementError);
	
	// Menu Resources
	sl_bool _parseMenuResource(const Ref<XmlElement>& element);
	Ref<SAppMenuResourceItem> _parseMenuResourceItem(const Ref<XmlElement>& element, SAppMenuResource* menu, int platforms);
	sl_bool _generateMenusCpp(const String& targetPath);
	sl_bool _generateMenusCpp_Item(const String& parentName, int parentPlatforms, SAppMenuResourceItem* item, StringBuffer& sbHeader, StringBuffer& sbCpp, int tabLevel);
	Ref<Menu> _getMenuValue(const SAppMenuValue& value);
	sl_bool _getMenuValue_Item(const Ref<Menu>& parent, SAppMenuResourceItem* item);
	sl_bool _checkMenuValueAvailable(SAppMenuValue& value, const Ref<XmlElement>& elementError);
	
	// Layout Resources
	sl_bool _parseLayoutStyle(const Ref<XmlElement>& element);
	sl_bool _parseLayoutInclude(const Ref<XmlElement>& element);
	sl_bool _parseLayoutResource(const Ref<XmlElement>& element);
	sl_bool _parseLayoutResourceItem(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent);
	Ref<SAppLayoutResourceItem> _parseLayoutResourceItemChild(SAppLayoutResource* layout, SAppLayoutResourceItem* parentItem, const Ref<XmlElement>& element);
	sl_bool _generateLayoutsCpp(const String& targetPath);
	sl_bool _generateLayoutsCppItem(SAppLayoutResource* layout, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent, StringBuffer& sbDeclare, StringBuffer& sbDefineInit, StringBuffer& sbDefineLayout, const String& addStatement);
	void _simulateLayoutInWindow(SAppLayoutResource* layout);
	void _registerLayoutSimulationWindow(const Ref<SAppLayoutSimulationWindow>& window);
	void _removeLayoutSimulationWindow(const Ref<SAppLayoutSimulationWindow>& window);
	Ref<View> _simulateLayoutCreateOrLayoutView(SAppLayoutSimulator* simulator, SAppLayoutResourceItem* item, SAppLayoutResourceItem* parent, View* parentView, sl_bool flagOnLayout);
	sl_ui_pos _getDimensionIntValue(SAppDimensionValue& value);
	sl_real _getDimensionFloatValue(SAppDimensionFloatValue& value);
	List< Ref<XmlElement> > _getLayoutItemChildElements(SAppLayoutResourceItem* item, const String& tagName);
	
	struct LayoutControlProcessParams
	{
		int op;
		SAppLayoutResource* resource;
		SAppLayoutResourceItem* resourceItem;
		SAppLayoutResourceItem* parentResourceItem;
		String name;
		String addStatement;
		StringBuffer* sbDeclare;
		StringBuffer* sbDefineInit;
		StringBuffer* sbDefineLayout;
		SAppLayoutSimulator* simulator;
		SAppLayoutSimulationWindow* window;
		Ref<View> view;
		View* parentView;
		sl_bool flagOnLayout;
	};
	sl_bool _processLayoutResourceControl(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Window(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Page(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_View(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_ViewGroup(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Import(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Button(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Label(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Check(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Radio(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Edit(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Password(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_TextArea(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Image(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Select(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Scroll(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Linear(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_List(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_ListReport(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Render(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Tab(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Tree(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Split(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Web(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Progress(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Slider(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Picker(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Pager(LayoutControlProcessParams* params);
	sl_bool _processLayoutResourceControl_Video(LayoutControlProcessParams* params);
	
private:
	sl_bool m_flagOpened;
	String m_pathConf;
	String m_pathApp;
	
	SAppConfiguration m_conf;
	
	TreeMap< String, Ref<SAppRawResource> > m_raws;
	TreeMap< String, Ref<SAppDrawableResource> > m_drawables;
	TreeMap< String, Ref<SAppStringResource> > m_strings;
	TreeMap< String, Ref<SAppMenuResource> > m_menus;
	HashMap< String, Ref<SAppLayoutStyle> > m_layoutStyles;
	HashMap< String, Ref<SAppLayoutInclude> > m_layoutIncludes;
	TreeMap< String, Ref<SAppLayoutResource> > m_layouts;
	CList< Ref<SAppLayoutSimulationWindow> > m_layoutSimulationWindows;
	SAppLayoutSimulationParams m_layoutSimulationParams;
	
	friend class SAppLayoutSimulationWindow;
	friend class SAppLayoutImportView;
	
};

SLIB_SDEV_NAMESPACE_END

#endif
