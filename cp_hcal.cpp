// cpp program to copy data auto to dCache
// Author Eldwan Brianne
//        DESY Hamburg
// August 2015
// workflow
// 1. Search in LOCALDIR for .last.RUNNUMBER
// 2. Copy files in DAQDIR with RUNNUMBER larger than the last
// 3. Copy using lcg-cr command
// 4. Check copy done correctly or try reupload

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

//Directory where are files to upload
string daq_dir = "INDAQ";
string local_dir = "INLOCAL";

//Root Directory on dCache to upload to 
string root_dCache = "ROOT";
string outdir_base = "BASE";
string outdir_tag = "TAG";
string outdir = "OUTDIR";

vector<string> v_filestoupload;
vector<string> v_filechecked;

bool LED = BOOLLED;
bool eudaq = BOOLEUDAQ;
bool bif = BOOLBIF;
bool verbose = VERBOSE;

bool CreatedCacheDirectory()
{

  cout << "Create dCache directory" << endl;

  string command = "lfc-mkdir -p ";
  command += root_dCache;
  command += outdir_base;
  command += outdir_tag;
  command += outdir;

  cout << command << endl;

  int status = system(command.c_str());

  if(status == 0)
    return true;
  else
    return false;
}

bool CopytodCache(string file)
{

  string srm = "dcache-se-desy.desy.de";
  string relative_path = outdir_base;
  relative_path += outdir_tag;
  relative_path += outdir;
  relative_path += file;

  string logical_filename = root_dCache;
  logical_filename += outdir_base;
  logical_filename += outdir_tag;
  logical_filename += outdir;
  logical_filename += file;

  string filepath = daq_dir;
  filepath += file;

  //cout << "SRM : " << srm << endl;
  //cout << "Relative Path : " << relative_path << " "; 
  //cout << "LFN : " << logical_filename << endl;
  //cout << "Filepath : " << filepath << endl;

  string command;
  if(verbose)
    command = "lcg-cr -v --vo calice -d ";
  else
    command = "lcg-cr --vo calice -d ";

  command += srm;
  command += " -P ";
  command += relative_path;
  command += " -l lfn:";
  command += logical_filename;
  command += " file://";
  command += filepath;

  cout << "Copy : " << command << endl;

  int status = system(command.c_str());
  //int status = 0;

  if(status == 0)
    return true;
  else
    return false;

}

//------------------------------------------------------

bool CheckFileondCache(string file)
{

  string command = "lfc-ls -l ";
  command += root_dCache;
  command += outdir_base;
  command += outdir_tag;
  command += outdir;
  command += file;

  int status = system(command.c_str());

  if(status == 0)
    return true;
  else
    return false;

}

//------------------------------------------------------

bool listFilesinDir(string dirname)
{

  if(dirname == " ")
    {
      cout << "daq_dir empty -- Plz provide it" << endl;
      exit(1);
    }

  cout << "List of file in dir : " << dirname << endl;
  
  system("rm listoffiles.txt");
  string list_command = "ls -l ";
  list_command += dirname;
  list_command += " | awk '{print $9}' > listoffiles.txt";

  cout << list_command << endl;
  
  int status = system(list_command.c_str());
  if(status == 0)
    return true;
  else
    return false;
}

//------------------------------------------------------

string CheckLastFileUploaded(string local_dir)
{

  struct dirent *de=NULL;
  DIR *d=NULL;
  d=opendir(local_dir.c_str());
  if(d == NULL)
    {
      perror("No local dir defined --- exiting");
      exit(1);
    }

  vector<string> veclistoffile;
  veclistoffile.clear();

  while(de = readdir(d))
    {
      veclistoffile.push_back(de->d_name);
    }

  closedir(d);

  //Getting last run number
  string last_run = " ";
  for(vector<string>::iterator it = veclistoffile.begin(); it != veclistoffile.end(); ++it)
    {
      if(strncmp((*it).c_str(), ".last", 4) == 0)
	{
	  last_run = *it;
	}
    }

  if(last_run == " ")
    {
      cout << "No file .last.RUNNUMBER was found --- Plz create one by doing touch .last.RUNUMBER exiting..." << endl;
      exit(1);
    }

  string last_run_number = last_run.substr(last_run.find("last.")+5, 5);
  return last_run_number;
}

//-----------------------------------------------------------

void ChangeLastRun(string last_run, string new_run)
{
  
  string command = "mv .last.";
  command += last_run;
  command += " .last.";
  command += new_run;
  
  system(command.c_str());

  cout << "New Run is " << new_run << endl;

  return;
}

//------------------------------------------------------------

void CheckFile(string last_run)
{

  ifstream fIn;
  fIn.open("listoffiles.txt");

  v_filestoupload.clear();

  //Checking files compared to last run number uploaded
  if(fIn.is_open())
    {
      string line;
      while(getline(fIn, line))
	{
	  if(line.empty())
	    continue;

	  istringstream iss(line);
	  string file;
	  iss >> file;

	  if(file == " ")
	    continue;

	  int run_num = 0;
	  
	  if(eudaq)
	    run_num = atoi((file.substr(file.find("Run_")+4, 5)).c_str());
	  else
	    run_num = atoi((file.substr(file.find("Run_")+7, 5)).c_str());
	  
	  if(run_num <= atoi(last_run.c_str()))
	    continue;
	  
	  //push back files with higher run number than last
	  v_filestoupload.push_back(file);

	}
      fIn.close();
    }
  else
    {
      cout << "Did not find listoffiles.txt --- exiting" << endl;
      exit(1);
    }

  //Checking time of the file and compare to time now
  for(vector<string>::iterator it = v_filestoupload.begin(); it != v_filestoupload.end(); ++it)
    {
      //Get file time last moification
      //format time ddpmmpyyyy__hhommoss
      struct stat attr;
      string path = daq_dir;
      path += (*it);
      
      stat(path.c_str(), &attr);
      struct tm *lt;
      lt = localtime(&attr.st_mtime);
      
      char month[50];
      char timestr[50];
      strftime(month, 50, "%d %b %Y", lt);
      strftime(timestr, 50, "%H:%M:%S", lt);
      
      cout << "------ Last File modification ------" << endl;
      cout << "This file is month : " << month << endl;
      cout << "This file is time : " << timestr << endl; 

      //Check if file is already on dCache
      bool ok = CheckFileondCache(*it);
      if(ok)
	{
	  cout << "File " << *it << " already on dCache" << endl;
	  continue;
	}
      else
	{	  
	  //Check if filesize != 0
	  size_t sizefile = attr.st_size;
	  if(sizefile == 0)
	    {
	      cout << "Size of the file null... -- not uploading" << endl;
	      continue;
	    }

	  cout << "File " << (*it) << " not on dCache --- upload ----" << endl;

	  sleep(1);

	  //if not try uploading
	  //if(time(0) - mktime(lt) > 2400)
	  if(time(0) - mktime(lt) > 5*60)
	    {
	      bool ok = CopytodCache(*it);
	      string new_run= " ";
	      if(eudaq)
		new_run = (*it).substr((*it).find("Run_")+4, 5);
	      else
		new_run = (*it).substr((*it).find("Run_")+7, 5);
	      
	      if(ok)
		{
		  cout << "File " << *it << " uploaded successfully!!!" << endl;
		  //Check if file is on dCache
		  bool ok = CheckFileondCache(*it);
		  
		  if(!ok)
		    {
		      cout << "Problem file " << *it << " not on dCache -- Try again" << endl;
		    }

		  ChangeLastRun(last_run, new_run);
		  last_run = new_run;
		}
	      else
		{
		  cout << "Problem with file " << *it << " -- Will try again!!" << endl;
		  ChangeLastRun(last_run, new_run);	
		  last_run = new_run;
		}
	    }
	  else
	    {
	      cout << "File too young to be uploaded" << endl;
	      cout << "Waiting more... Diff time : " << time(0) - mktime(lt) << endl;
	      continue;
	    }
	}
    }
  
  return;
  
}

//-----------------------------------------

void CopyLEDfiles()
{
  ifstream fIn;
  fIn.open("listoffiles.txt");

  v_filestoupload.clear();

  //Push back files into vector
  if(fIn.is_open())
    {
      string line;
      while(getline(fIn, line))
	{
	  if(line.empty())
	    continue;

	  istringstream iss(line);
	  string file;
	  iss >> file;
	  
	  if(file == " ")
	    continue;

	  v_filestoupload.push_back(file);
	}
      fIn.close();
    }
  else
    {
      cout << "Did not find listoffiles.txt --- exiting" << endl;
      exit(1);
    }
  
  for(vector<string>::iterator it = v_filestoupload.begin(); it != v_filestoupload.end(); ++it)
    {

      struct stat attr;
      string path = daq_dir;
      path += (*it);
      
      stat(path.c_str(), &attr);
      struct tm *lt;
      lt = localtime(&attr.st_mtime);

      char month[28];
      char timestr[28];
      strftime(month, 28, "%d %b %Y", lt);
      strftime(timestr, 28, "%H:%M:%S", lt);
     
      cout << "------ Last File modification ------" << endl;
      cout << "This file is month : " << month << endl;
      cout << "This file is time : " << timestr << endl; 

      
      //Check if file is already on dCache
      bool ok = CheckFileondCache(*it);
      if(ok)
	{
	  cout << "File " << *it << " already on dCache" << endl;
	  continue;
	}
      else
	{
	  if(time(0) - mktime(lt) > 2400)
	    {
	      cout << "File " << (*it) << " not on dCache --- upload ----" << endl;
	      
	      sleep(5);
	      
	      bool ok = CopytodCache(*it);
	      
	      if(ok)
		{
		  cout << "File " << *it << " uploaded successfully!!!" << endl;
		  //Check if file is on dCache
		  bool ok = CheckFileondCache(*it);
		  
		  if(!ok)
		    {
		      cout << "Problem file " << *it << " not on dCache -- Try again" << endl;
		    }
		}
	      else
		{
		  cout << "Problem with file " << *it << " -- Will try again!!" << endl;
		} 
	    }
	  else
	    {
	      cout << "File too young for uploading" << endl;
	    }
	}
    }
  return;
}

//------------------------------------------

void CopyBIFfiles()
{
  ifstream fIn;
  fIn.open("listoffiles.txt");

  v_filestoupload.clear();

  //Push back files into vector
  if(fIn.is_open())
    {
      string line;
      while(getline(fIn, line))
	{
	  if(line.empty())
	    continue;

	  istringstream iss(line);
	  string file;
	  iss >> file;
	  
	  if(file == " ")
	    continue;

	  v_filestoupload.push_back(file);
	}
      fIn.close();
    }
  else
    {
      cout << "Did not find listoffiles.txt --- exiting" << endl;
      exit(1);
    }
  
  for(vector<string>::iterator it = v_filestoupload.begin(); it != v_filestoupload.end(); ++it)
    {

      struct stat attr;
      string path = daq_dir;
      path += (*it);
      
      stat(path.c_str(), &attr);
      struct tm *lt;
      lt = localtime(&attr.st_mtime);

      char month[28];
      char timestr[28];
      strftime(month, 28, "%d %b %Y", lt);
      strftime(timestr, 28, "%H:%M:%S", lt);
     
      cout << "------ Last File modification ------" << endl;
      cout << "This file is month : " << month << endl;
      cout << "This file is time : " << timestr << endl; 

      
      //Check if file is already on dCache
      bool ok = CheckFileondCache(*it);
      if(ok)
	{
	  cout << "File " << *it << " already on dCache" << endl;
	  continue;
	}
      else
	{
	  if(time(0) - mktime(lt) > 2400)
	    {
	      cout << "File " << (*it) << " not on dCache --- upload ----" << endl;
	      
	      sleep(5);
	      
	      bool ok = CopytodCache(*it);
	      
	      if(ok)
		{
		  cout << "File " << *it << " uploaded successfully!!!" << endl;
		  //Check if file is on dCache
		  bool ok = CheckFileondCache(*it);
		  
		  if(!ok)
		    {
		      cout << "Problem file " << *it << " not on dCache -- Try again" << endl;
		    }
		}
	      else
		{
		  cout << "Problem with file " << *it << " -- Will try again!!" << endl;
		} 
	    }
	  else
	    {
	      cout << "File too young for uploading" << endl;
	    }
	}
    }
  return;
}

//------------------------------------------

void SplashScreen()
{

  cout << "##########################################################" << endl;
  cout << "                                                          " << endl;
  cout << "      Tool to upload to dCache automatically              " << endl;
  cout << "      Author : Eldwan Brianne (DESY)                      " << endl;
  cout << "      Date : August 2015                                  " << endl;
  cout << "      email : eldwan.brianne@desy.de                      " << endl;
  cout << "                                                          " << endl;
  cout << "##########################################################" << endl;

}


//--------------------------------------------

int main()
{

  SplashScreen();

  if(LED && !bif)
    {
      if(CreatedCacheDirectory())
	{
	  cout << "Copy LED files..." << endl;
	  while(1)
	    {
	      if(listFilesinDir(daq_dir))
		{
		  CopyLEDfiles();
		  sleep(30);
		}
	      else
		{
		  cout << "Could not get listoffiles in daq_dir : " << daq_dir << " exiting! -- Plz verify it!" << endl;
		  exit(1);
		}
	    }
	}
      else
	{
	  cout << "Could not create directory on dCache --- Check proxy validity!!!" << endl;
	  exit(1);
	}
    }

  if(bif && !LED)
    {
      if(CreatedCacheDirectory())
	{
	  cout << "Copy BIF files..." << endl;
	  while(1)
	    {
	      if(listFilesinDir(daq_dir))
		{
		  CopyBIFfiles();
		  sleep(30);
		}
	      else
		{
		  cout << "Could not get listoffiles in daq_dir : " << daq_dir << " exiting! -- Plz verify it!" << endl;
		  exit(1);
		}
	    }
	}
      else
	{
	  cout << "Could not create directory on dCache --- Check proxy validity!!!" << endl;
	  exit(1);
	}
    }

  if(!bif && !LED)
    {
      if(CreatedCacheDirectory())
	{
	  while(1)
	    {
	      if(listFilesinDir(daq_dir))
		{
		  string last_run = CheckLastFileUploaded(local_dir);
		  cout << "Last Run is " << last_run << endl;
		  CheckFile(last_run);

		  sleep(30);
		}
	      else
		{
		  cout << "Could not get listoffiles in daq_dir : " << daq_dir << " exiting! -- Plz verify it!" << endl;
		  exit(1);
		}
	    }
	}
      else
	{
	  cout << "Could not create directory on dCache --- Check proxy validity!!!" << endl;
	  exit(1);
	}
    }

  return 0;
}
