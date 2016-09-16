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
		ListItems<String> radioGroups(layout->radioGroups.keys());
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
		setSize(800, 450);
		setResizable(sl_true);
		viewContent = new ViewGroup;
		m_simulationContentView = viewContent;
	}
	viewContent = doc->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_false);
	if (viewContent.isNotNull()) {
		if (layout->type != SAppLayoutResource::typeWindow) {
			addView(viewContent);
		}
		doc->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_true);
		create();
		doc->_registerLayoutSimulationWindow(this);
	}
}

void SAppLayoutSimulationWindow::layoutViews(sl_ui_len width, sl_ui_len height)
{
	Ref<SAppDocument> doc = m_document;
	Ref<SAppLayoutResource> layout = m_layoutResource;
	if (doc.isNotNull() && layout.isNotNull()) {
		doc->_simulateLayoutCreateOrLayoutView(this, layout.ptr, sl_null, sl_true);
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
		ListItems<String> radioGroups(layout->radioGroups.keys());
		for (sl_size i = 0; i < radioGroups.count; i++) {
			Ref<RadioGroup> group = new RadioGroup;
			if (group.isNotNull()) {
				m_radioGroups.put(radioGroups[i], group);
			}
		}
	}
	m_simulationContentView = this;
	Ref<View> viewContent = document->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_false);
	if (viewContent.isNotNull()) {
		document->_simulateLayoutCreateOrLayoutView(this, layout, sl_null, sl_true);
	}
}

void SAppLayoutImportView::layoutViews(sl_ui_len width, sl_ui_len height)
{
	Ref<SAppDocument> doc = m_document;
	Ref<SAppLayoutResource> layout = m_layoutResource;
	if (doc.isNotNull() && layout.isNotNull()) {
		doc->_simulateLayoutCreateOrLayoutView(this, layout.ptr, sl_null, sl_true);
	}
}


SLIB_SDEV_NAMESPACE_END
