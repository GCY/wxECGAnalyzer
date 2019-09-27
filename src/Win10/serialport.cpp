#include "serialport.h"

SerialPort::SerialPort()
{
}

SerialPort::SerialPort(const char *path,int baud_rate = 9600,int databits = 8,int stopbits = 1,int parity = 'N')
{
   this->path = path;
   this->baud_rate = baud_rate;
   this->databits = databits;
   this->stopbits = stopbits;
   this->parity = parity;
}

SerialPort::~SerialPort()
{
#ifdef _WIN_
   if (com){
      CloseHandle(com);//Closing the Serial Port
   }
#elif _MAC_
   Close();
#endif
}

#ifdef _WIN_ 
bool SerialPort::Open(const std::wstring path)
#elif _MAC_
bool SerialPort::Open(const char *path = "")
#endif
{
#ifdef _WIN_
   if (com){
      CloseHandle(com);//Closing the Serial Port
   }

   int path_len = path.length();
   com_path = std::wstring(L"");
   memset(&dcb, 0, sizeof(dcb)); // Initializing DCB structure

   if (path_len > 4) { //Not COM1~COM9
      com_path.append(L"\\\\.\\");
      com_path.append(path);
   }
   else { //COM1~COM9
      com_path.append(path);
   }

   std::string com_path_str(com_path.begin(), com_path.end());
   com = CreateFile(com_path_str.c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, 0, NULL);

   if (com == INVALID_HANDLE_VALUE) {
      return false;
   }
#elif _MAC_
   if(device){
      close(device);
   }

   device = open(path,O_RDWR);

   if(device == -1){
      return false;
   }
#endif

   return true;
}

bool SerialPort::SetBaudRate(int baud_rate = 9600)
{
#ifdef _WIN_
   const int speed_arr[] = { CBR_115200,CBR_57600,CBR_38400,CBR_19200,CBR_9600,CBR_4800,CBR_2400,CBR_1200,CBR_300 };
   const int name_arr[] = { 115200,57600,38400,19200,9600,4800,2400,1200,300 };
   DCB dcb = { 0 }; // Initializing DCB structure

   dcb.DCBlength = sizeof(dcb);

   bool status = GetCommState(com, &dcb);
   if (!status){
      return false;
   }

   for (int i = 0; i < sizeof(speed_arr) / sizeof(int); ++i) {
      if (baud_rate == name_arr[i]) {
	 dcb.BaudRate = speed_arr[i];  
      }

      status = SetCommState(com, &dcb);
   }

   if(!status){
      return false;
   }

   // Set comm timeout parameter
   memset(&timeouts, 0, sizeof(timeouts));
   timeouts.ReadIntervalTimeout = MAXDWORD; // in milliseconds
   timeouts.ReadTotalTimeoutConstant = 0;//50; // in milliseconds
   timeouts.ReadTotalTimeoutMultiplier = 0;//10; // in milliseconds
   timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
   timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds

   //Set interval timeout for wave detect
   status = SetCommTimeouts(com,&timeouts); 
   if (!status){
      return false;
   }

   status = PurgeComm(com,PURGE_RXCLEAR); // clear input buffer
   if (!status){
      return false;
   }

#elif _MAC_
   const int speed_arr[] = {B115200,B57600,B38400,B19200,B9600,B4800,B2400,B1200,B300};
   const int name_arr[] = {115200,57600,38400,19200,9600,4800,2400,1200,300};

   struct termios options;

   tcgetattr(device,&options);

   for(int i = 0; i < sizeof(speed_arr) / sizeof(int);++i){
      if(baud_rate == name_arr[i]){
	 tcflush(device,TCIOFLUSH);
	 cfsetispeed(&options,speed_arr[i]);
	 cfsetospeed(&options,speed_arr[i]);
	 tcsetattr(device,TCSANOW,&options);
      }
      if(tcflush(device,TCIOFLUSH) != 0){
	 return false;
      }
   }
#endif
   return true;
}

bool SerialPort::SetParity(int databits = 8,int stopbits = 1,int parity = 'N')
{
#ifdef _WIN_
   unsigned char com_parity;
   unsigned char com_stopbits;
   DCB dcb = { 0 }; // Initializing DCB structure

   dcb.DCBlength = sizeof(dcb);

   bool status = GetCommState(com, &dcb);
   if (!status){
      return false;
   }

   switch (parity)
   {
      case 'N':case 'n':
	 {
	    com_parity = NOPARITY;
	    break;
	 }
      case 'O':case 'o':
	 {
	    com_parity = ODDPARITY;
	    break;
	 }
      case 'E':case 'e':
	 {
	    com_parity = EVENPARITY;
	    break;
	 }
      case 'S':case 's':
	 {
	    com_parity = SPACEPARITY;
	    break;
	 }
      default:	
	 {
	    return false;
	 }
   }

   switch (stopbits)
   {
      case 1:
	 {
	    com_stopbits = ONESTOPBIT;
	    break;
	 }
      case 2:
	 {
	    com_stopbits = TWOSTOPBITS;
	    break;
	 }
      default:
	 {
	    return false;
	 }
   }

   dcb.ByteSize = databits;
   dcb.Parity = com_parity;
   dcb.StopBits = com_stopbits;

   status = SetCommState(com, &dcb);
   if (!status){
      return false;
   }

   // Set comm timeout parameter
   memset(&timeouts, 0, sizeof(timeouts));
   timeouts.ReadIntervalTimeout = MAXDWORD; // in milliseconds
   timeouts.ReadTotalTimeoutConstant = 0;//50; // in milliseconds
   timeouts.ReadTotalTimeoutMultiplier = 0;//10; // in milliseconds
   timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
   timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds

   //Set interval timeout for wave detect
   status = SetCommTimeouts(com, &timeouts); 
   if (!status){
      return false;
   }
   status = PurgeComm(com, PURGE_RXCLEAR); // clear input buffer
   if (!status){
      return false;
   }

#elif _MAC_
   struct termios options;

   if(tcgetattr(device,&options) != 0){
      return false;
   }

   options.c_cflag &= ~CSIZE;

   switch(databits)
   {
      case 7:
	 {
	    options.c_cflag |= CS7;
	    break;
	 }
      case 8:
	 {
	    options.c_cflag |= CS8;
	    break;
	 }
      default:
	 {
	    return false;
	 }
   }

   switch(parity)
   {
      case 'n':case 'N':
	 {
	    options.c_cflag &= ~PARENB;
	    options.c_iflag &= ~INPCK;
	    break;
	 }
      case 'o':case 'O':
	 {
	    options.c_cflag |= (PARODD | PARENB);
	    options.c_iflag |= INPCK;
	    break;
	 }
      case 'e':case 'E':
	 {
	    options.c_cflag |= PARENB;
	    options.c_cflag &= ~PARODD;
	    options.c_iflag |= INPCK;
	    break;
	 }
      case 's':case 'S':
	 {
	    options.c_cflag &= ~PARENB;
	    options.c_cflag &= ~CSTOPB;
	    break;
	 }
      default:
	 {
	    return false;
	 }
   }

   switch(stopbits)
   {
      case 1:
	 {
	    options.c_cflag &= ~CSTOPB;
	    break;
	 }
      case 2:
	 {
	    options.c_cflag |= CSTOPB;
	    break;
	 }
      default:
	 {
	    return false;
	 }
   }

   if(parity != 'n' || parity != 'N'){
      options.c_iflag |= INPCK;
   }

   options.c_cc[VTIME] = 150;
   options.c_cc[VMIN] = 0;

   tcflush(device,TCIFLUSH);

   if(tcsetattr(device,TCSANOW,&options) != 0){
      return false;
   }
#endif
   return true;
}

void SerialPort::Close()
{
#ifdef _WIN_
   if (com){
      CloseHandle(com);//Closing the Serial Port
	  com = NULL;
   }
#elif _MAC_
   if(device){
      close(device);
   }
#endif
}

void SerialPort::Write(unsigned char *data,int length)
{
#ifdef _WIN_
   unsigned long bytes_towrite = length; // No of bytes to write into the port
   unsigned long bytes_written = 0;   // No of bytes written to the port

   bool status = WriteFile(com, data, bytes_towrite, &bytes_written, NULL);

#elif _MAC_
   if(device){
      write(device,data,length);
   }
#endif
}

void SerialPort::Read(unsigned char *data,int length)
{
#ifdef _WIN_ 
   unsigned long len_toread = length;
   unsigned long bytes_read;
   bool status = ReadFile(com, data, len_toread, &bytes_read, NULL);
#elif _MAC_
   if(device){
      read(device,data,length);
   }
#endif
}

int SerialPort::Read(unsigned char *data)
{
#ifdef _WIN_ 
   unsigned long len_toread =0;
   unsigned long bufferlength = 0;
   unsigned long bytes_read;
   len_toread = 30000;

   // Determine the number of new bytes.
   COMSTAT comstat;
   unsigned long errormask = 0;

   ClearCommError(com, &errormask, &comstat);
   bufferlength = comstat.cbInQue;
   if (!bufferlength) { return -1; }
   if (bufferlength < 30000){
      len_toread = bufferlength;
   }

   bool status = ReadFile(com, data, len_toread, &bytes_read, NULL);
   if(bytes_read == len_toread){
      return bytes_read;
   }
#elif _MAC_
   if(device == -1){
      return -1;
   }
   int length;
   int io_status = ioctl(device,FIONREAD,&length);
   if(io_status == -1){
      Close();
      return -1;
   }
   if(length > 0){
      // If the number of bytes read is equal to the number of bytes retrieved
      if(read(device,data,length) == length){  
	 return length;
      }  
   }
#endif
   return -1;
}
