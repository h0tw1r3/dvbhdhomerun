#ifndef _conf_inifile_h_
#define _conf_inifile_h_

#include <string>
#include <map>

typedef std::map<std::string, std::string> mapKeyVal;
typedef std::map<std::string, mapKeyVal > mapSecKeyVal;

class ConfIniFile
{
public:
  bool OpenIniFile(const std::string& _fileName);
  bool GetSecValue(const std::string& _section, const std::string& _key,
		   std::string& _value);

private:

private:
  mapSecKeyVal m_sectionKeyValue;
};

#endif // _conf_inifile_h_
