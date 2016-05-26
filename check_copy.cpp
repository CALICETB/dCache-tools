// cpp program to check the data copied on the dCache
// Author Eldwan Brianne
//        DESY Hamburg
// August 2015
// workflow
// 1. Get listoffiles in DAQDIR 
// 2. Get listoffiles on dCache DIR
// 3. Check lists
// 4. Copy if missing file

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
#include <algorithm>

using namespace std;

//Directory where are files to upload
string daq_dir = "INDAQ";
string local_dir = "INLOCAL";

//Root Directory on dCache to upload to 
string root_dCache = "ROOT";
string outdir_base = "BASE";
string outdir_tag = "TAG";
string outdir = "OUTDIR";

vector<string> v_localfiles;
vector<string> v_dCachefiles;

bool Copy2dCache(string file)
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

  /* Old lcg command
     string command = "lcg-cr -v --vo calice -d ";
     command += srm;
     command += " -P ";
     command += relative_path;
     command += " -l lfn:";
     command += logical_filename;
     command += " file://";
     command += filepath;
  */

  /* New gfal command */
  string command;
  if(verbose)
    command = "gfal-copy -v ";
  else
    command = "gfal-copy ";

  command += "file://";
  command += filepath;
  command += " srm://";
  command += srm;
  command += "/pnfs/desy.de/calice/";
  command += relative_path;
  command += " lfn:";
  command += logical_filename;
  
  cout << "Copy : " << command << endl;

  int status = system(command.c_str());
  //int status = 0;

  if(status == 0)
    return true;
  else
    return false;
}

bool GetListofLocalFiles()
{
  string dirname = daq_dir;

  if(dirname == " ")
    {
      cout << "daq_dir empty -- Plz provide it" << endl;
      exit(1);
    }
  
  string list_command = "ls -l ";
  list_command += dirname;
  list_command += " | awk '{print $9}' > listoffiles_local.txt";
  
  int status = system(list_command.c_str());
  
  if(status == 0)
    {
      ifstream fIn;
      fIn.open("listoffiles_local.txt");

      v_localfiles.clear(); 

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
	      
	      //push back files with higher run number than last
	      v_localfiles.push_back(file);   
	    }
	  
	  fIn.close();
	  cout << "Got list of local files" << endl;

	  return true;
	}
      else
	{
	  cout << "Did not find listoffiles_local.txt --- exiting" << endl;
	  exit(1);
	}   
    }
  else
    {
      cout << "Could not execute ls -l command" << endl;
      return false;
    }
}

bool GetListofdCacheFiles()
{
  /* Old lfc command 
  string command = "lcg-ls -l ";
  command += "lfn:";
  command += root_dCache;
  command += outdir_base;
  command += outdir_tag;
  command += outdir;
  command += " | awk '{print $6}' > listoffiles_dcache.txt";
  */

  /* New gfal command */
  string command = "gfal-ls -l ";
  command += "srm://";
  command += srm;
  command += "/pnfs/desy.de/calice/";
  command += outdir_base;
  command += outdir_tag;
  command += outdir;
  command += " | awk '{print $9}' > listoffiles_dcache.txt";
  
  int status = system(command.c_str());
  
  if(status == 0)
    {
      ifstream fIn;
      fIn.open("listoffiles_dcache.txt");

      v_dCachefiles.clear(); 

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
	      
	      if(file == "EUDAQ")
		continue;
	      
	      //push back files with higher run number than last
	      v_dCachefiles.push_back(file);   
	    }
	  
	  fIn.close();
	  cout << "Got list of dCache files" << endl;
	  return true;
	}
      else
	{
	  cout << "Did not find listoffiles_dcache.txt --- exiting" << endl;
	  exit(1);
	}   
    }
  else
    {
      cout << "Could not execute lcg-ls -l command" << endl;
      return false;
    }
}

void CompareFiles()
{
  
  cout << "Start comparison ...." << endl;

  if(v_localfiles.empty())
    {
      cout << "Vector of local files empty...." << endl;
    }
  
  if(v_dCachefiles.empty())
    {
      cout << "Vector of dCache files empty...." << endl;
    }

  if(v_localfiles.empty() || v_dCachefiles.empty())
    {
      cout << "One of the vector is empty" << endl;
      return;
    }

  vector<string>::const_iterator itLocal;  
  for(itLocal = v_localfiles.begin(); itLocal != v_localfiles.end(); ++itLocal)
    {
      vector<string>::const_iterator it_dCache = find(v_dCachefiles.begin(), v_dCachefiles.end(), (*itLocal)); 
 
      if (it_dCache != v_dCachefiles.end())
	{
	  cout << "Found file '" << *it_dCache << "' on the dCache." << endl;
	}
      else
	{
	  cout << "Need to copy " << *it_dCache << " to dCache" << endl;
	  bool ok = Copy2dCache(*it_dCache);
	  if(ok)
	    {
	      cout << "File " << *it_dCache << " copied successfully" << endl;
	    }
	  else
	    {
	      cout << "[WARNING] File " << *it_dCache << " NOT COPIED TO DCACHE!!!!" << endl;
	    }
	}
    }

  return;
}

int main()
{

  if(GetListofLocalFiles())
    {
      if(GetListofdCacheFiles())
	{
	  CompareFiles();
	}
    }

  cout << "Check Done... exiting" << endl;

  return 0;
}

