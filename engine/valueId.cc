/*
  @copyright Steve Keen 2021
  @author Russell Standish
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "plotWidget.h"
#include "valueId.h"
#include "variableValue.h"
#include "group.h"
#include "lasso.h"
#include "latexMarkup.h"
#include "minsky_epilogue.h"

#include <boost/locale.hpp>
using namespace boost::locale::conv;
using namespace std;

namespace minsky
{
  bool isValueId(const string& name)
  {
    // original code, left for reference
    //    static regex pattern("((constant)?\\d*:[^:\\ \f\n\r\t\v]+)");
    //    return name.length()>1 && name.substr(name.length()-2)!=":_" &&
    //      regex_match(utf_to_utf<char>(name), pattern);   // Leave curly braces in valueIds. For ticket 1165
    if (name.substr(name.length()-2)==":_") return false;
    
    static string constantPrefix="constant:", tempPrefix="temp:";
    auto nameCStr=name.c_str();
    const char* endp=nullptr;
    strtoull(nameCStr,&const_cast<char*&>(endp),10);
    if (*endp==':' || name.starts_with(constantPrefix)||name.starts_with(tempPrefix))
      {
        if (*endp!=':')
          {
            if (name[constantPrefix.length()-1]==':')
              endp=nameCStr+constantPrefix.length()-1;
            else
              endp=nameCStr+tempPrefix.length()-1;
          }
        // check unqualified name portion has no verboten characters
        for (auto c=endp+1; *c!='\0'; ++c)
          if (strchr(":\\ \f\n\r\t\v",*c))
            return false;
        return true;
      }
    return false;
  }

  string canonicalName(const string& name)
  {
    return utf_to_utf<char>(stripActive(trimWS(latexToPangoNonItalicised(uqName(name)))));
  }

  string valueIdCanonical(size_t scope, const string& name)
  {
    auto tmp=":"+name;
    if (scope==0) return tmp;
    return to_string(scope)+tmp;
  }

  string valueId(const string& name)
  {
    return valueIdCanonical(scope(name), canonicalName(name));
  }

  string valueId(const GroupPtr& ref, const string& name) 
  {return valueIdFromScope(scope(ref,utf_to_utf<char>(name)), canonicalName(name));}

  size_t scope(const string& name) 
  {
    if (name.starts_with("temp:")) return 0; // temporaries are "global"
    auto nm=utf_to_utf<char>(name);
    auto nameCStr=nm.c_str();
    char* endp=nullptr;
    const size_t r=strtoull(nameCStr,&endp,10);
    if (endp && *endp==':')
      return r;
    throw error("scope requested for local variable");
    // old implementation left for reference
    // smatch m;
    //    if (regex_search(nm, m, regex(R"((\d*)]?:.*)")))
    //      if (m.size()>1 && m[1].matched && !m[1].str().empty())
    //        return stoull(m[1].str());
    //      else
    //        return 0;
    //    else
    //      // no scope information is present
    //      throw error("scope requested for local variable");
  }

  GroupPtr scope(GroupPtr scope, const string& a_name)
  {
    auto name=canonicalName(a_name);
    if (a_name[0]==':' && scope)
      {
        // find maximum enclosing scope that has this same-named variable
        for (auto g=scope->group.lock(); g; g=g->group.lock())
          for (auto& i: g->items)
            if (auto v=i->variableCast())
              {
                if (v->canonicalName()==name)
                  {
                    scope=g;
                    goto break_outerloop;
                  }
              }
        scope.reset(); // global var
      break_outerloop: ;
      }
    return scope;
  }
  
  
  string valueIdFromScope(const GroupPtr& scope, const std::string& name)
  {
    if (name.empty() || !scope || !scope->group.lock())
      return valueIdCanonical(0,name); // retain previous global var id
    return valueIdCanonical(size_t(scope.get()), name);
}
  
  std::string uqName(const std::string& name)
  {
    const string::size_type p=name.rfind(':');
    if (p==string::npos)
      return utf_to_utf<char>(name);
    return utf_to_utf<char>(name).substr(p+1);
  }
 
}
