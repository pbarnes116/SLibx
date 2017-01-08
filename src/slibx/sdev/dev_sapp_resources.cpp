#include "../../../inc/slibx/sdev/sapp.h"
#include <slib/ui.h>

SLIB_SDEV_NAMESPACE_BEGIN

Ref<Image> SAppDrawableResourceImageItem::loadImage()
{
	ObjectLocker lock(this);
	if (image.isNull()) {
		image = Image::loadFromFile(filePath);
	}
	return image;
}

SAppDrawableResourceImageAttributes::SAppDrawableResourceImageAttributes()
{
	defaultImages = List< Ref<SAppDrawableResourceImageItem> >::create();
}

SAppDrawableResource::SAppDrawableResource()
{
	type = typeUnknown;
}


SAppLayoutViewAttributes::SAppLayoutViewAttributes()
{
	finalFontBold = sl_false;
	finalFontItalic = sl_false;
	finalFontUnderline = sl_false;
}

sl_bool SAppLayoutViewAttributes::isNotRequiredNative()
{
	if (backgroundColor.flagDefined) {
		return sl_true;
	}
	if (background.flagDefined) {
		return sl_true;
	}
	if (pressedBackground.flagDefined) {
		return sl_true;
	}
	if (hoverBackground.flagDefined) {
		return sl_true;
	}
	if (borderWidth.flagDefined) {
		return sl_true;
	}
	if (borderColor.flagDefined) {
		return sl_true;
	}
	if (borderStyle.flagDefined) {
		return sl_true;
	}
	return sl_false;
}


sl_bool SAppLayoutButtonAttributes::isNotRequiredNative()
{
	if (textColor.flagDefined) {
		return sl_true;
	}
	if (icon.flagDefined) {
		return sl_true;
	}
	if (iconWidth.flagDefined) {
		return sl_true;
	}
	if (iconHeight.flagDefined) {
		return sl_true;
	}
	if (gravity.flagDefined) {
		return sl_true;
	}
	if (iconAlign.flagDefined) {
		return sl_true;
	}
	if (textAlign.flagDefined) {
		return sl_true;
	}
	if (textBeforeIcon.flagDefined) {
		return sl_true;
	}
	if (orientation.flagDefined) {
		return sl_true;
	}
	if (iconMarginLeft.flagDefined) {
		return sl_true;
	}
	if (iconMarginTop.flagDefined) {
		return sl_true;
	}
	if (iconMarginRight.flagDefined) {
		return sl_true;
	}
	if (iconMarginBottom.flagDefined) {
		return sl_true;
	}
	if (textMarginLeft.flagDefined) {
		return sl_true;
	}
	if (textMarginTop.flagDefined) {
		return sl_true;
	}
	if (textMarginRight.flagDefined) {
		return sl_true;
	}
	if (textMarginBottom.flagDefined) {
		return sl_true;
	}
	if (defaultColorFilter.flagDefined) {
		return sl_true;
	}
	for (sl_uint32 i = 0; i < SLIB_SAPP_LAYOUT_BUTTON_CATEGORY_MAX; i++) {
		SAppLayoutButtonCategory& category = categories[i];
		for (sl_uint32 k = 0; k < (sl_uint32)(ButtonState::Count); k++) {
			if (category.textColor[k].flagDefined) {
				return sl_true;
			}
			if (category.icon[k].flagDefined) {
				return sl_true;
			}
			if (category.background[k].flagDefined) {
				return sl_true;
			}
			if (category.borderWidth[k].flagDefined || category.borderColor[k].flagDefined || category.borderStyle[k].flagDefined) {
				return sl_true;
			}
		}
	}
	return sl_false;
}


sl_bool SAppLayoutLabelAttributes::isNotRequiredNative()
{
	return sl_false;
}


sl_bool SAppLayoutEditAttributes::isNotRequiredNative()
{
	return sl_false;
}


sl_bool SAppLayoutSelectAttributes::isNotRequiredNative()
{
	if (leftIcon.flagDefined) {
		return sl_true;
	}
	if (rightIcon.flagDefined) {
		return sl_true;
	}
	if (iconWidth.flagDefined) {
		return sl_true;
	}
	if (iconHeight.flagDefined) {
		return sl_true;
	}
	if (textColor.flagDefined) {
		return sl_true;
	}
	return sl_false;
}


sl_bool SAppLayoutTabAttributes::isNotRequiredNative()
{
	if (orientation.flagDefined) {
		return sl_true;
	}
	if (tabWidth.flagDefined) {
		return sl_true;
	}
	if (tabWidth.flagDefined) {
		return sl_true;
	}
	if (barBackground.flagDefined) {
		return sl_true;
	}
	if (contentBackground.flagDefined) {
		return sl_true;
	}
	if (tabBackground.flagDefined) {
		return sl_true;
	}
	if (selectedTabBackground.flagDefined) {
		return sl_true;
	}
	if (hoverTabBackground.flagDefined) {
		return sl_true;
	}
	if (labelColor.flagDefined) {
		return sl_true;
	}
	if (selectedLabelColor.flagDefined) {
		return sl_true;
	}
	if (hoverLabelColor.flagDefined) {
		return sl_true;
	}
	if (labelAlign.flagDefined) {
		return sl_true;
	}
	if (labelMarginLeft.flagDefined) {
		return sl_true;
	}
	if (labelMarginTop.flagDefined) {
		return sl_true;
	}
	if (labelMarginRight.flagDefined) {
		return sl_true;
	}
	if (labelMarginBottom.flagDefined) {
		return sl_true;
	}
	return sl_false;
}

sl_bool SAppLayoutPickerAttributes::isNotRequiredNative()
{
	if (textColor.flagDefined) {
		return sl_true;
	}
	return sl_false;
}


SAppLayoutResourceItem::SAppLayoutResourceItem()
{
	type = typeUnknown;
	flagGeneratedName = sl_false;
}

String SAppLayoutResourceItem::getXmlAttribute(const String& name)
{
	String value = element->getAttribute(name);
	if (value.isNotNull()) {
		return value;
	}
	ListLocker< Ref<SAppLayoutStyle> > _styles(styles);
	for (sl_size i = 0; i < _styles.count; i++) {
		Ref<SAppLayoutStyle> style = _styles[i];
		if (style.isNotNull()) {
			value = style->element->getAttribute(name);
			if (value.isNotNull()) {
				return value;
			}
		}
	}
	return String::null();
}

SAppLayoutResource::SAppLayoutResource()
{
	nAutoIncreaseNameView = 0;
	nAutoIncreaseNameViewGroup = 0;
	nAutoIncreaseNameImport = 0;
	nAutoIncreaseNameButton = 0;
	nAutoIncreaseNameLabel = 0;
	nAutoIncreaseNameCheck = 0;
	nAutoIncreaseNameRadio = 0;
	nAutoIncreaseNameEdit = 0;
	nAutoIncreaseNamePassword = 0;
	nAutoIncreaseNameTextArea = 0;
	nAutoIncreaseNameImage = 0;
	nAutoIncreaseNameSelect = 0;
	nAutoIncreaseNameScroll = 0;
	nAutoIncreaseNameLinear = 0;
	nAutoIncreaseNameList = 0;
	nAutoIncreaseNameListReport = 0;
	nAutoIncreaseNameRender = 0;
	nAutoIncreaseNameTab = 0;
	nAutoIncreaseNameTree = 0;
	nAutoIncreaseNameWeb = 0;
	nAutoIncreaseNameSplit = 0;
	nAutoIncreaseNameProgress = 0;
	nAutoIncreaseNameSlider = 0;
	nAutoIncreaseNamePicker = 0;
	nAutoIncreaseNamePager = 0;
	nAutoIncreaseNameOther = 0;
}

String SAppLayoutResource::getAutoIncreasingName(int type)
{
	String prefix;
	sl_uint32* pN;
	switch (type) {
		case typeView:
			prefix = "view";
			pN = &nAutoIncreaseNameView;
			break;
		case typeViewGroup:
			prefix = "group";
			pN = &nAutoIncreaseNameViewGroup;
			break;
		case typeImport:
			prefix = "import";
			pN = &nAutoIncreaseNameImport;
			break;
		case typeButton:
			prefix = "button";
			pN = &nAutoIncreaseNameButton;
			break;
		case typeLabel:
			prefix = "label";
			pN = &nAutoIncreaseNameLabel;
			break;
		case typeCheck:
			prefix = "check";
			pN = &nAutoIncreaseNameCheck;
			break;
		case typeRadio:
			prefix = "radio";
			pN = &nAutoIncreaseNameRadio;
			break;
		case typeEdit:
			prefix = "edit";
			pN = &nAutoIncreaseNameEdit;
			break;
		case typePassword:
			prefix = "password";
			pN = &nAutoIncreaseNamePassword;
			break;
		case typeTextArea:
			prefix = "textArea";
			pN = &nAutoIncreaseNameTextArea;
			break;
		case typeImage:
			prefix = "image";
			pN = &nAutoIncreaseNameImage;
			break;
		case typeSelect:
			prefix = "select";
			pN = &nAutoIncreaseNameSelect;
			break;
		case typeScroll:
			prefix = "scroll";
			pN = &nAutoIncreaseNameScroll;
			break;
		case typeLinear:
			prefix = "linear";
			pN = &nAutoIncreaseNameLinear;
			break;
		case typeList:
			prefix = "list";
			pN = &nAutoIncreaseNameList;
			break;
		case typeListReport:
			prefix = "listReport";
			pN = &nAutoIncreaseNameListReport;
			break;
		case typeRender:
			prefix = "render";
			pN = &nAutoIncreaseNameRender;
			break;
		case typeTab:
			prefix = "tab";
			pN = &nAutoIncreaseNameTab;
			break;
		case typeTree:
			prefix = "tree";
			pN = &nAutoIncreaseNameTree;
			break;
		case typeWeb:
			prefix = "web";
			pN = &nAutoIncreaseNameWeb;
			break;
		case typeSplit:
			prefix = "split";
			pN = &nAutoIncreaseNameSplit;
			break;
		case typeProgress:
			prefix = "progress";
			pN = &nAutoIncreaseNameProgress;
			break;
		case typeSlider:
			prefix = "slider";
			pN = &nAutoIncreaseNameSlider;
			break;
		case typePicker:
			prefix = "picker";
			pN = &nAutoIncreaseNamePicker;
			break;
		case typePager:
			prefix = "pager";
			pN = &nAutoIncreaseNamePager;
			break;
		default:
			prefix = "other";
			pN = &nAutoIncreaseNameOther;
			break;
	}
	while (1) {
		(*pN)++;
		String name = String::format("_%s%d", prefix, *pN);
		if (!(itemsByName.contains(name))) {
			return name;
		}
	}
	return String::null();
}

int SAppLayoutResource::getTypeFromName(const String &strType)
{
	int type = SAppLayoutResource::typeUnknown;
	if (strType == "view") {
		type = SAppLayoutResource::typeView;
	} else if (strType == "group") {
		type = SAppLayoutResource::typeViewGroup;
	} else if (strType == "import") {
		type = SAppLayoutResource::typeImport;
	} else if (strType == "button") {
		type = SAppLayoutResource::typeButton;
	} else if (strType == "label") {
		type = SAppLayoutResource::typeLabel;
	} else if (strType == "check") {
		type = SAppLayoutResource::typeCheck;
	} else if (strType == "radio") {
		type = SAppLayoutResource::typeRadio;
	} else if (strType == "edit") {
		type = SAppLayoutResource::typeEdit;
	} else if (strType == "password") {
		type = SAppLayoutResource::typePassword;
	} else if (strType == "textArea") {
		type = SAppLayoutResource::typeTextArea;
	} else if (strType == "image") {
		type = SAppLayoutResource::typeImage;
	} else if (strType == "select") {
		type = SAppLayoutResource::typeSelect;
	} else if (strType == "scroll") {
		type = SAppLayoutResource::typeScroll;
	} else if (strType == "linear") {
		type = SAppLayoutResource::typeLinear;
	} else if (strType == "list") {
		type = SAppLayoutResource::typeList;
	} else if (strType == "list-report") {
		type = SAppLayoutResource::typeListReport;
	} else if (strType == "render") {
		type = SAppLayoutResource::typeRender;
	} else if (strType == "tab") {
		type = SAppLayoutResource::typeTab;
	} else if (strType == "tree") {
		type = SAppLayoutResource::typeTree;
	} else if (strType == "web") {
		type = SAppLayoutResource::typeWeb;
	} else if (strType == "split") {
		type = SAppLayoutResource::typeSplit;
	} else if (strType == "progress") {
		type = SAppLayoutResource::typeProgress;
	} else if (strType == "slider") {
		type = SAppLayoutResource::typeSlider;
	} else if (strType == "picker") {
		type = SAppLayoutResource::typePicker;
	} else if (strType == "pager") {
		type = SAppLayoutResource::typePager;
	}
	return type;
}


SAppLayoutSimulationParams::SAppLayoutSimulationParams()
{
	screenWidth = 0;
	screenHeight = 0;
	viewportWidth = 0;
	viewportHeight = 0;
	sp = 1;
}


Ref<Referable> SAppLayoutSimulator::getReferable()
{
	return m_refer;
}

Ref<View> SAppLayoutSimulator::getViewByName(const String& name)
{
	return m_views.getValue(name, Ref<View>::null());
}

void SAppLayoutSimulator::registerViewByName(const String& name, const Ref<View>& view)
{
	m_views.put(name, view);
}

Ref<RadioGroup> SAppLayoutSimulator::getRadioGroup(const String& name)
{
	return m_radioGroups.getValue(name, Ref<RadioGroup>::null());
}

Ref<SAppDocument> SAppLayoutSimulator::getDocument()
{
	return m_document;
}

Ref<SAppLayoutResource> SAppLayoutSimulator::getLayoutResource()
{
	return m_layoutResource;
}

Ref<SAppLayoutSimulationWindow> SAppLayoutSimulator::getSimulationWindow()
{
	return m_simulationWindow;
}

Ref<View> SAppLayoutSimulator::getSimulationContentView()
{
	return m_simulationContentView;
}

SAppLayoutSimulationWindow::SAppLayoutSimulationWindow()
{
	SLIB_REFERABLE_CONSTRUCTOR
	m_simulationWindow = this;
	m_refer = this;
}

void SAppLayoutSimulationWindow::open(SAppDocument* doc, SAppLayoutResource* layout)
{
	m_document = doc;
	m_layoutResource = layout;
	{
		ListElements<String> radioGroups(layout->radioGroups.getAllKeys());
		for (sl_size i = 0; i < radioGroups.count; i++) {
			Ref<RadioGroup> group = new RadioGroup;
			if (group.isNotNull()) {
				m_radioGroups.put(radioGroups[i], group);
			}
		}
	}
	Ref<View> viewContent;
	if (layout->type == SAppLayoutResource::typeWindow) {
		m_simulationContentView = getContentView();
	} else {
		setCenterScreenOnCreate(sl_true);
		setSize(450, 800);
		setResizable(sl_true);
		viewContent = new ViewGroup;
		m_simulationContentView = viewContent;
	}
	viewContent = doc->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_null, sl_false);
	setInitialized();
	if (viewContent.isNotNull()) {
		if (layout->type != SAppLayoutResource::typeWindow) {
			addView(viewContent);
		}
		doc->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_null, sl_true);
		create();
		doc->_registerLayoutSimulationWindow(this);
	}
}

void SAppLayoutSimulationWindow::layoutViews(sl_ui_len width, sl_ui_len height)
{
	Ref<SAppDocument> doc = m_document;
	Ref<SAppLayoutResource> layout = m_layoutResource;
	if (doc.isNotNull() && layout.isNotNull()) {
		doc->_simulateLayoutCreateOrLayoutView(this, layout.get(), sl_null, sl_null, sl_true);
	} 
}

void SAppLayoutSimulationWindow::onClose(UIEvent* ev)
{
	Ref<SAppDocument> doc = m_document;
	if (doc.isNotNull()) {
		doc->_removeLayoutSimulationWindow(this);
	}
}


SAppLayoutImportView::SAppLayoutImportView()
{
	SLIB_REFERABLE_CONSTRUCTOR
	m_refer = this;
}

void SAppLayoutImportView::init(SAppLayoutSimulator* simulator, SAppLayoutResource* layout)
{
	Ref<SAppDocument> document = simulator->getDocument();
	m_document = document;
	m_simulationWindow = simulator->getSimulationWindow();
	m_layoutResource = layout;
	{
		ListElements<String> radioGroups(layout->radioGroups.getAllKeys());
		for (sl_size i = 0; i < radioGroups.count; i++) {
			Ref<RadioGroup> group = new RadioGroup;
			if (group.isNotNull()) {
				m_radioGroups.put(radioGroups[i], group);
			}
		}
	}
	m_simulationContentView = this;
	Ref<View> viewContent = document->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_null, sl_false);
	setInitialized();
	if (viewContent.isNotNull()) {
		document->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_null, sl_true);
	}
}

void SAppLayoutImportView::layoutViews(sl_ui_len width, sl_ui_len height)
{
	Ref<SAppDocument> doc = m_document;
	Ref<SAppLayoutResource> layout = m_layoutResource;
	if (doc.isNotNull() && layout.isNotNull()) {
		doc->_simulateLayoutCreateOrLayoutView(this, layout.get(), sl_null, sl_null, sl_true);
	}
}


SLIB_SDEV_NAMESPACE_END
