#include "enumserial.h"

EnumSerial::EnumSerial()
{
}

std::vector<std::string> EnumSerial::EnumSerialPort()
{
   std::vector<std::string> paths;
#ifdef _WIN_ 
   //search com   

   if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hkey ))
   {
      unsigned long index = 0;
      long return_value;

      while ((return_value = RegEnumValue(hkey, index, key_valuename, &len_valuename, 0,
		  &key_type, (LPBYTE)key_valuedata,&len_valuedata)) == ERROR_SUCCESS)
      {
	 if (key_type == REG_SZ)
	 {		
		std::wstring key_valuedata_ws(key_valuedata);
	    std::string key_valuedata_str(key_valuedata_ws.begin(), key_valuedata_ws.end());
	    paths.push_back(key_valuedata_str);
	 }
	 len_valuename = 1000;
	 len_valuedata = 1000;
	 index++;
      }
   }
   // close key
   RegCloseKey(hkey);
#elif _MAC_
   root = std::string("/dev/");
   DIR *dp;
   struct dirent *dirp;
   if((dp  = opendir(root.c_str())) == NULL) {
   }
   while ((dirp = readdir(dp)) != NULL) {
      std::string filename(dirp->d_name);
      if(filename.find(std::string("cu.")) != std::string::npos){
	 paths.push_back(std::string(dirp->d_name));
	 continue;
      }
      if(filename.find(std::string("tty.")) != std::string::npos){
	 paths.push_back(std::string(dirp->d_name));
	 continue;
      }
   }
   closedir(dp);   
#endif  

   return paths;
}
