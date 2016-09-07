#ifndef CHECKHEADER_SLIB_SDEV_SAPP_RESOURCES
#define CHECKHEADER_SLIB_SDEV_SAPP_RESOURCES

#include "definition.h"

#include "sapp_values.h"

#include <slib/core/locale.h>
#include <slib/core/xml.h>
#include <slib/graphics/image.h>
#include <slib/ui/event.h>
#include <slib/ui/resource.h>

SLIB_SDEV_NAMESPACE_BEGIN

class SAppStringResource : public Referable
{
public:
	String name;
	String defaultValue;
	HashMap<Locale, String> values;
};

class SAppRawResource : public Referable
{
public:
	String name;
	String path;
	String filePath;
};

class SAppDrawableResourceImageItem : public Object
{
public:
	String fileName;
	String filePath;
	String rawName;
	SafeRef<Image> image;
	
public:
	Ref<Image> loadImage();
	
};


class SAppDrawableResourceImageAttributes : public Referable
{
public:
	List< Ref<SAppDrawableResourceImageItem> > defaultImages;
	HashMap< Locale, List< Ref<SAppDrawableResourceImageItem> > > images;

public:
	SAppDrawableResourceImageAttributes();
	
};

class SAppDrawableResourceNinePiecesAttributes : public Referable
{
public:
	Ref<XmlElement> element;
	
	SAppDimensionValue leftWidth;
	SAppDimensionValue rightWidth;
	SAppDimensionValue topHeight;
	SAppDimensionValue bottomHeight;
	
	SAppDrawableValue topLeft;
	SAppDrawableValue top;
	SAppDrawableValue topRight;
	SAppDrawableValue left;
	SAppDrawableValue center;
	SAppDrawableValue right;
	SAppDrawableValue bottomLeft;
	SAppDrawableValue bottom;
	SAppDrawableValue bottomRight;
	
};

class SAppDrawableResourceNinePatchAttributes : public Referable
{
public:
	Ref<XmlElement> element;
	
	SAppDimensionValue dstLeftWidth;
	SAppDimensionValue dstRightWidth;
	SAppDimensionValue dstTopHeight;
	SAppDimensionValue dstBottomHeight;
	
	SAppDrawableValue src;
	sl_real leftWidth;
	sl_real rightWidth;
	sl_real topHeight;
	sl_real bottomHeight;
	
};

class SAppDrawableResource : public Referable
{
public:
	String name;
	int type;
	enum {
		typeUnknown = 0,
		typeImage = 1,
		typeNinePieces = 2,
		typeNinePatch = 3
	};
	
	Ref<SAppDrawableResourceImageAttributes> imageAttrs;
	Ref<SAppDrawableResourceNinePiecesAttributes> ninePiecesAttrs;
	Ref<SAppDrawableResourceNinePatchAttributes> ninePatchAttrs;
	
public:
	SAppDrawableResource();
	
};


class SAppMenuResourceItem : public Referable
{
public:
	Ref<XmlElement> element;
	String name;
	int type;
	enum {
		typeSubmenu = 0,
		typeItem = 1,
		typeSeparator = 2
	};
	int platformFlags;
	enum {
		mac = 1,
		windows = 2,
		linux = 4,
		all_platforms = 0xFFFF,
		no_mac = 0xFFFE,
		no_windows = 0xFFFD,
		no_linux = 0xFFFB
	};
	SAppStringValue title;
	KeycodeAndModifiers shortcutKey;
	KeycodeAndModifiers macShortcutKey;
	SAppDrawableValue icon;
	SAppDrawableValue checkedIcon;
	CList< Ref<SAppMenuResourceItem> > children;
};

class SAppMenuResource : public Referable
{
public:
	String name;
	CList< Ref<SAppMenuResourceItem> > children;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsWindows;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsLinux;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsMac;
};


class SAppLayoutResourceItem;

class SAppLayoutWindowAttributes : public Referable
{
public:
	SAppMenuValue menu;
	SAppStringValue title;
	
	SAppDimensionValue left;
	SAppDimensionValue top;
	SAppDimensionValue width;
	SAppDimensionValue height;
	
	SAppBooleanValue minimized;
	SAppBooleanValue maximized;
	SAppBooleanValue visible;
	SAppBooleanValue alwaysOnTop;
	SAppBooleanValue closeButton;
	SAppBooleanValue minimizeButton;
	SAppBooleanValue maximizeButton;
	SAppBooleanValue resizable;
	SAppFloatValue alpha;
	SAppBooleanValue transparent;
	
	SAppBooleanValue modal;
	SAppBooleanValue dialog;
	SAppBooleanValue borderless;
	SAppBooleanValue titleBar;
	SAppBooleanValue fullScreen;
	SAppBooleanValue centerScreen;
};

class SAppLayoutViewAttributes : public Referable
{
public:
	SAppStringValue _id;
	
	SAppDimensionValue width;
	SAppDimensionValue height;

	PositionMode leftMode;
	PositionMode topMode;
	PositionMode rightMode;
	PositionMode bottomMode;
	String leftReferingView;
	String topReferingView;
	String rightReferingView;
	String bottomReferingView;
	
	SAppDimensionValue left;
	SAppDimensionValue top;

	SAppDimensionValue marginLeft;
	SAppDimensionValue marginTop;
	SAppDimensionValue marginRight;
	SAppDimensionValue marginBottom;

	SAppDimensionValue paddingLeft;
	SAppDimensionValue paddingTop;
	SAppDimensionValue paddingRight;
	SAppDimensionValue paddingBottom;
	
	SAppVisibilityValue visibility;
	SAppBooleanValue enabled;
	SAppBooleanValue opaque;
	SAppBooleanValue occurringClick;
	
	SAppDrawableValue background;
	SAppScaleModeValue backgroundScale;
	SAppAlignmentValue backgroundAlign;
	SAppColorValue backgroundColor;
	SAppDimensionValue borderWidth;
	SAppColorValue borderColor;
	SAppPenStyleValue borderStyle;
	
	SAppStringValue fontFamily;
	SAppDimensionValue fontSize;
	SAppBooleanValue fontBold;
	SAppBooleanValue fontItalic;
	SAppBooleanValue fontUnderline;
	SAppStringValue finalFontFamily;
	SAppDimensionValue finalFontSize;
	sl_bool finalFontBold;
	sl_bool finalFontItalic;
	sl_bool finalFontUnderline;
	
	SAppScrollBarsValue scrollBars;
	SAppBooleanValue scrollingByMouse;
	SAppBooleanValue scrollingByTouch;
	SAppBooleanValue scrollingByMouseWheel;
	SAppBooleanValue scrollingByKeyboard;
	
	SAppBooleanValue multiTouch;
	SAppBooleanValue tabStop;

	SAppBooleanValue instance;
	SAppBooleanValue childInstances;
	SAppBooleanValue nativeWidget;
	SAppBooleanValue doubleBuffering;
	
public:
	SAppLayoutViewAttributes();

};

class SAppLayoutButtonCategory
{
public:
	SAppColorValue textColor[(int)(ButtonState::Count)];
	SAppColorValue backgroundColor[(int)(ButtonState::Count)];
	SAppDrawableValue background[(int)(ButtonState::Count)];
	SAppDrawableValue icon[(int)(ButtonState::Count)];
	SAppDimensionValue borderWidth[(int)(ButtonState::Count)];
	SAppColorValue borderColor[(int)(ButtonState::Count)];
	SAppPenStyleValue borderStyle[(int)(ButtonState::Count)];
	
};

#define SLIB_SAPP_LAYOUT_BUTTON_CATEGORY_MAX 4

class SAppLayoutButtonAttributes : public Referable
{
public:
	SAppStringValue text;
	SAppBooleanValue defaultButton;
	SAppColorValue textColor;
	SAppDrawableValue icon;
	SAppDimensionValue iconWidth;
	SAppDimensionValue iconHeight;
	SAppAlignmentValue gravity;
	SAppAlignmentValue iconAlign;
	SAppAlignmentValue textAlign;
	SAppBooleanValue textBeforeIcon;
	SAppLayoutOrientationValue orientation;
	SAppDimensionValue iconMarginLeft;
	SAppDimensionValue iconMarginTop;
	SAppDimensionValue iconMarginRight;
	SAppDimensionValue iconMarginBottom;
	SAppDimensionValue textMarginLeft;
	SAppDimensionValue textMarginTop;
	SAppDimensionValue textMarginRight;
	SAppDimensionValue textMarginBottom;
	
	SAppLayoutButtonCategory categories[SLIB_SAPP_LAYOUT_BUTTON_CATEGORY_MAX];
	
};

class SAppLayoutLabelAttributes : public Referable
{
public:
	SAppStringValue text;
	SAppColorValue textColor;
	SAppAlignmentValue gravity;
	
};

class SAppLayoutCheckAttributes : public Referable
{
public:
	SAppBooleanValue checked;
	
};

class SAppLayoutRadioAttributes : public Referable
{
public:
	String group;
	
};

class SAppLayoutEditAttributes : public Referable
{
public:
	SAppStringValue text;
	SAppAlignmentValue gravity;
	SAppStringValue hintText;
	SAppBooleanValue readOnly;
	SAppBooleanValue multiLine;
	SAppColorValue textColor;
	
};

class SAppLayoutImageAttributes : public Referable
{
public:
	SAppDrawableValue src;
	SAppScaleModeValue scale;
	SAppAlignmentValue gravity;
	
};

class SAppLayoutSelectItem
{
public:
	SAppStringValue title;
	SAppStringValue value;
	SAppBooleanValue selected;
};

class SAppLayoutSelectAttributes : public Referable
{
public:
	SAppDimensionValue iconWidth;
	SAppDimensionValue iconHeight;
	SAppDrawableValue leftIcon;
	SAppDrawableValue rightIcon;
	SAppColorValue textColor;
	
	CList<SAppLayoutSelectItem> items;
	
};

class SAppLayoutScrollAttributes : public Referable
{
public:
	SAppScrollingValue scrolling;
	
	Ref<SAppLayoutResourceItem> content;
	
};

class SAppLayoutLinearAttributes : public Referable
{
public:
	SAppLayoutOrientationValue orientation;
	
};

class SAppLayoutListAttributes : public Referable
{
public:
};

struct SAppLayoutListReportColumn
{
	SAppStringValue title;
	SAppDimensionValue width;
	SAppAlignmentValue align;
	SAppAlignmentValue headerAlign;
};

class SAppLayoutListReportAttributes : public Referable
{
public:
	CList<SAppLayoutListReportColumn> columns;
	
};

class SAppLayoutStyle : public Referable
{
public:
	Ref<XmlElement> element;
	String name;
};

class SAppLayoutResourceItem : public Referable
{
public:
	enum {
		typeUnknown = 0,
		typeWindow = 0x0100,
		typeMobilePage = 0x0101,
		typeView = 0x0200,
		typeViewGroup = 0x0201,
		typeButton = 0x0202,
		typeLabel = 0x0203,
		typeCheck = 0x0204,
		typeRadio = 0x0205,
		typeEdit = 0x0206,
		typePassword = 0x0207,
		typeTextArea = 0x0208,
		typeImage = 0x0210,
		typeSelect = 0x0211,
		typeScroll = 0x0212,
		typeLinear = 0x0213,
		typeList = 0x0214,
		typeListReport = 0x0215,
		typeRender = 0x0220,
		typeTab = 0x0221,
		typeTree = 0x0222,
		typeWeb = 0x0223
	};
	
	Ref<XmlElement> element;
	
	int type;
	String name;
	sl_bool flagGeneratedName;
	
	String className;
	
	Ref<SAppLayoutWindowAttributes> windowAttrs;
	Ref<SAppLayoutViewAttributes> viewAttrs;
	Ref<SAppLayoutButtonAttributes> buttonAttrs;
	Ref<SAppLayoutLabelAttributes> labelAttrs;
	Ref<SAppLayoutCheckAttributes> checkAttrs;
	Ref<SAppLayoutRadioAttributes> radioAttrs;
	Ref<SAppLayoutEditAttributes> editAttrs;
	Ref<SAppLayoutImageAttributes> imageAttrs;
	Ref<SAppLayoutSelectAttributes> selectAttrs;
	Ref<SAppLayoutScrollAttributes> scrollAttrs;
	Ref<SAppLayoutLinearAttributes> linearAttrs;
	Ref<SAppLayoutListAttributes> listAttrs;
	Ref<SAppLayoutListReportAttributes> listReportAttrs;

	CList< Ref<SAppLayoutStyle> > styles;
	CList< Ref<SAppLayoutResourceItem> > children;
	
public:
	SAppLayoutResourceItem();
	
public:
	String getXmlAttribute(const String& name);
	
	List< Ref<XmlElement> > getChildElements(const String& name);
	
};

class SAppLayoutResource : public SAppLayoutResourceItem
{
public:
	SAppDimensionValue customUnit;
	
	HashMap< String, Ref<SAppLayoutResourceItem> > itemsByName;
	TreeMap<String, sl_bool> customClasses;
	TreeMap<String, sl_bool> radioGroups;

	sl_uint32 nAutoIncreaseNameView;
	sl_uint32 nAutoIncreaseNameViewGroup;
	sl_uint32 nAutoIncreaseNameButton;
	sl_uint32 nAutoIncreaseNameLabel;
	sl_uint32 nAutoIncreaseNameCheck;
	sl_uint32 nAutoIncreaseNameRadio;
	sl_uint32 nAutoIncreaseNameEdit;
	sl_uint32 nAutoIncreaseNamePassword;
	sl_uint32 nAutoIncreaseNameTextArea;
	sl_uint32 nAutoIncreaseNameImage;
	sl_uint32 nAutoIncreaseNameSelect;
	sl_uint32 nAutoIncreaseNameScroll;
	sl_uint32 nAutoIncreaseNameLinear;
	sl_uint32 nAutoIncreaseNameList;
	sl_uint32 nAutoIncreaseNameListReport;
	sl_uint32 nAutoIncreaseNameRender;
	sl_uint32 nAutoIncreaseNameTab;
	sl_uint32 nAutoIncreaseNameTree;
	sl_uint32 nAutoIncreaseNameWeb;
	sl_uint32 nAutoIncreaseNameOther;
	
public:
	SAppLayoutResource();
	
public:
	String getAutoIncreasingName(int type);
	
	static int getTypeFromName(const String& strType);
	
};

class SAppLayoutSimulationParams
{
public:
	sl_real screenWidth;
	sl_real screenHeight;
	sl_real viewportWidth;
	sl_real viewportHeight;
	sl_real customUnit;
	
public:
	SAppLayoutSimulationParams();
	
};

class SAppDocument;

class SAppLayoutSimulationWindow : public WindowLayoutResource
{
public:
	SAppLayoutSimulationWindow();
	
public:
	void open(SAppDocument* doc, SAppLayoutResource* layout);
	
	Ref<View> getViewByName(const String& name);
	
	void registerViewByName(const String& name, const Ref<View>& view);
	
	Ref<RadioGroup> getRadioGroup(const String& name);
	
	Ref<SAppDocument> getDocument();
	
	Ref<SAppLayoutResource> getLayout();

protected:
	// override
	void layoutViews(sl_real width, sl_real height);
	
	// override
	void onClose(UIEvent* ev);
	
protected:
	SafeWeakRef<SAppDocument> m_document;
	SafeRef<SAppLayoutResource> m_layout;
	HashMap< String, Ref<View> > m_views;
	HashMap< String, Ref<RadioGroup> > m_radioGroups;
	
};

SLIB_SDEV_NAMESPACE_END

#endif
