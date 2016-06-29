#ifndef CHECKHEADER_SLIB_SDEV_SAPP
#define CHECKHEADER_SLIB_SDEV_SAPP

#include "definition.h"

#include <slib/core/resource.h>
#include <slib/core/xml.h>
#include <slib/ui/event.h>

SLIB_SDEV_NAMESPACE_BEGIN

class SAppConfiguration
{
public:
	String generate_cpp_target_path;
	String generate_cpp_namespace;
	
};

class SAppStringResource : public Referable
{
public:
	String name;
	String defaultValue;
	HashMap<Locale, String> values;
};

class SAppStringValue
{
public:
	sl_bool flagReferResource;
	String value;
};

class SAppMenuResourceItem : public Referable
{
public:
	String name;
	sl_uint32 platformFlags;
	enum {
		mac = 1,
		windows = 2,
		linux = 4
	};
	SAppStringValue title;
	KeycodeAndModifiers shortcutKey;
	KeycodeAndModifiers macShortcutKey;
	CList< Ref<SAppMenuResourceItem> > children;
};

class SAppMenuResource : public Referable
{
public:
	String name;
	CList< Ref<SAppMenuResourceItem> > children;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsWindows;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsNoWindows;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsLinux;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsNoLinux;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsMac;
	HashMap<String, Ref<SAppMenuResourceItem> > itemsNoMac;
};

class SAppDocument : public Object
{
public:
	SAppDocument();
	
public:
	sl_bool open(const String& filePath);
	
	void close();
	
	sl_bool isOpened();
	
	sl_bool openResources();
	
	sl_bool generateCpp();

protected:
	sl_bool _parseConfiguration(const String& filePath, SAppConfiguration& conf);
	
	void _freeResources();
	
	sl_bool _parseResources(const String& filePath);
	
	sl_bool _parseStringResources(const Ref<XmlElement>& element);
	
	sl_bool _parseStringResource(const Ref<XmlElement>& element, Locale locale);
	
	sl_bool _parseMenuResource(const Ref<XmlElement>& element);
	
	sl_bool _generateStringsCpp(const String& targetPath);

	void _log(const String& text);

	void _logError(const String& text);
	
	void _logError(sl_uint32 line, sl_uint32 col, const String& text);
	
	void _logError(const Ref<XmlElement>& element, const String& text);
	
private:
	sl_bool m_flagOpened;
	String m_pathConf;
	String m_pathApp;
	
	SAppConfiguration m_conf;
	
	TreeMap< String, Ref<SAppStringResource> > m_strings;
	TreeMap< String, Ref<SAppMenuResource> > m_menus;
	
	String m_lastFilePath;
	
};

SLIB_SDEV_NAMESPACE_END

#endif
