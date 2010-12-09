#include "conf_inifile.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

bool ConfIniFile::OpenIniFile(const string& _filename)
{
  ifstream conffile;
  conffile.open(_filename.c_str(), ios::in);
  if(conffile.is_open()) {
    string line;
    string section;
    while(getline(conffile, line)) {
      if(line.empty()) {
	//cout << " ignore, empty";
      } 
      else if(strchr(line.c_str(), '#') != NULL) {
	//cout << " ignore, comment";
      }
      else if(strchr(line.c_str(), '[') != NULL) {
	section = line.substr(1, line.size() - 2);
	//cout << " section: " << section;
      }
      else if(strchr(line.c_str(), '=') != NULL) {
	string key;
	string value;
	istringstream str(line);
	getline(str, key, '=');
	getline(str, value);
	//cout << "key:" << key << " value:" << value << " ";
	m_sectionKeyValue.insert( make_pair(section, map<string, string>()) );
	m_sectionKeyValue[section].insert( make_pair(key, value));
      }
      else {
	//cout << " ignore, unknown";
      }

      //cout << line << ":" << line.size();
      //cout << endl;
    }

    return true;
  }

  return false;
}

bool ConfIniFile::GetSecValue(const std::string& _section, const std::string& _key,
			      std::string& _value)
{
  mapSecKeyVal::iterator it = m_sectionKeyValue.find(_section);

  if( it != m_sectionKeyValue.end() ) {
    mapKeyVal::iterator keyValIt = it->second.find(_key);

    if( keyValIt != it->second.end() ) {
      _value = keyValIt->second;
      return true;
    }
  }

  return false;
}

