// Resource.hh
// Copyright (c) 2002-2003 Henrik Kinnunen (fluxgen(at)users.sourceforge.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// $Id: Resource.hh,v 1.7 2003/12/19 18:25:39 fluxgen Exp $

#ifndef FBTK_RESOURCE_HH
#define FBTK_RESOURCE_HH

#include "NotCopyable.hh"
#include "XrmDatabaseHelper.hh"
#include <string>
#include <list>
#include <iostream>

#include <X11/Xlib.h>
#include <X11/Xresource.h>

namespace FbTk {

class XrmDatabaseHelper;

/// Base class for resources, this is only used in ResourceManager
class Resource_base:private FbTk::NotCopyable
{
public:
    virtual ~Resource_base() { };	
	
    /// set from string value
    virtual void setFromString(char const *strval) = 0;
    /// set default value
    virtual void setDefaultValue() = 0;
    /// get string value
    virtual std::string getString() = 0;
    /// get alternative name of this resource
    inline const std::string& altName() const { return m_altname; }
    /// get name of this resource
    inline const std::string& name() const { return m_name; }

protected:	
    Resource_base(const std::string &name, const std::string &altname):
	m_name(name), m_altname(altname)
	{ }

private:
    std::string m_name; ///< name of this resource
    std::string m_altname; ///< alternative name 
};

template <typename T>
class Resource;

class ResourceManager
{
public:
    typedef std::list<Resource_base *> ResourceList;

    // lock specifies if the database should be opened with one level locked
    // (useful for constructing inside initial set of constructors)
    ResourceManager(const char *filename, bool lock_db);
    virtual ~ResourceManager();

    /// Load all resources registered to this class
    /// @return true on success
    virtual bool load(const char *filename);

    /// Save all resouces registered to this class
    /// @return true on success
    virtual bool save(const char *filename, const char *mergefilename=0);

    

    /// Add resource to list, only used in Resource<T>
    template <class T>
    void addResource(Resource<T> &r);

    /// Remove a specific resource, only used in Resource<T>
    template <class T>
    void removeResource(Resource<T> &r) {
        m_resourcelist.remove(&r);
    }

    Resource_base *findResource(const std::string &resname);
    std::string resourceValue(const std::string &resname);
    void setResourceValue(const std::string &resname, const std::string &value);

    // this marks the database as "in use" and will avoid reloading 
    // resources unless it is zero.
    // It returns this resource manager. Useful for passing to 
    // constructors like Object(m_rm.lock())
    ResourceManager &lock();
    void unlock();
    // for debugging
    inline int lockDepth() const { return m_db_lock; }
    void dump(bool show_value = false) {
        ResourceList::iterator it = m_resourcelist.begin();
        ResourceList::iterator it_end = m_resourcelist.end();
        for (; it != it_end; ++it) {
            if (show_value)
                std::cout<<(*it)->name()<<": "<<(*it)->getString()<<std::endl;
            else
                std::cout<<(*it)->name()<<std::endl;
        }
    }
protected:
    static void ensureXrmIsInitialize();

    int m_db_lock;

private:
    static bool m_init;
    ResourceList m_resourcelist;

    XrmDatabaseHelper *m_database;

    std::string m_filename;
};


/// Real resource class
/**
 * usage: Resource<int> someresource(resourcemanager, 10, "someresourcename", "somealternativename"); \n
 * and then implement setFromString and getString \n
 * example: \n
 * template <> \n
 * void Resource<int>::setFromString(const char *str) { \n
 *   *(*this) = atoi(str); \n
 * }
 */
template <typename T>
class Resource:public Resource_base
{
public:	
    Resource(ResourceManager &rm, T val, 
             const std::string &name, const std::string &altname):
	Resource_base(name, altname),
	m_value(val), m_defaultval(val),
	m_rm(rm)
	{
            m_rm.addResource(*this); // add this to resource handler
	}
    virtual ~Resource() {
        m_rm.removeResource(*this); // remove this from resource handler
    }

    inline void setDefaultValue() override {  m_value = m_defaultval; }
    /// sets resource from string, specialized, must be implemented
    void setFromString(const char *strval) override;
    inline Resource<T>& operator = (const T& newvalue) { m_value = newvalue;  return *this;}
    /// specialized, must be implemented
    /// @return string value of resource
    std::string getString() override;

    inline T& operator*() { return m_value; }
    inline const T& operator*() const { return m_value; }
    inline T *operator->() { return &m_value; }
    inline const T *operator->() const { return &m_value; }
private:
    T m_value, m_defaultval;
    ResourceManager &m_rm;
};


// add the resource and load its value
template <class T>
void ResourceManager::addResource(Resource<T> &r) {
    m_resourcelist.push_back(&r);
    m_resourcelist.unique();

    // lock ensures that the database is loaded.
    lock();

    if (m_database == 0) {
        unlock();
        return;
    }

    XrmValue value;
    char *value_type;

    // now, load the value for this resource
    if (XrmGetResource(**m_database, r.name().c_str(),
                       r.altName().c_str(), &value_type, &value)) {
        r.setFromString(value.addr);
    } else {
        std::cerr<<"Failed to read: "<<r.name()<<std::endl;
        std::cerr<<"Setting default value"<<std::endl;
        r.setDefaultValue();
    }

    unlock();
}
	

} // end namespace FbTk

#endif // FBTK_RESOURCE_HH
