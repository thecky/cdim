#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <iterator>
#include <iomanip>	// for setw/setfill
#include <cerrno>
#include <cstring>

#include "libcdim.h"

using namespace std;

#define MIN_TRACK 1
#define MAX_TRACK 40
#define MIN_SECTOR 0
#define MAX_SECTOR 22

void print_usage (const string& errormsg = "")
{
  cout << "Usage: cdim [options] -f imagefilename" << endl << endl;
  cout << "Options:" << endl;
  cout << "\t-h, --help\tprint usage and exit" << endl;
  cout << "\t-c, --create\tcreate an new imagefile" << endl;
  cout << "\t-d, --directory\tshow directory from image" << endl;
  cout << "\t-u, --dump\tdump a block (requires -s and -t)" << endl;
  cout << endl;
  cout << "\t-f, --filename\timagefilename (required!)" << endl;
  cout << "\t-s, --sector\tsector parameter (f.e. for --dump)" << endl;
  cout << "\t-t, --track\ttrack parameter (f.e. for --dump)" << endl;
  
  if (!errormsg.empty())
  {
    cout << endl;
    cout << errormsg << endl;
  }
}

int main (int argc, char *argv[])
{
  string imagefilename = "";
  unsigned int track, sector;
  
  bool opt_dir, opt_create, opt_dump;
  opt_dir = false;
  opt_create = false;
  opt_dump = false;
  
  bool valid_track, valid_sector, require_ts;
  valid_track = false;
  valid_sector = false;
  require_ts = false;
  
  bool valid_filename;
  valid_filename = false;
  
  if (argc < 3) { print_usage(); return false; }
  
  for (int i = 1; i < argc; ++i)
  {
    string arg = argv[i];
    
    /* check for help option */
    if (arg == "-h" || arg == "--help")
    {
      print_usage(); return true;
    }
    
    /* check for track parameter */
    if (arg == "-t" || arg == "--track")
    {
      if (i + 1 < argc)
      {
	string para = argv[++i];
	istringstream (para) >> track;
	
	if (track < MIN_TRACK || track > MAX_TRACK)
	{
	  string msg = "Trackvalue out of range";
	  print_usage (msg);
	  return false;
	}
	
	valid_track = true;
      }
    }

    /* check for sector parameter */
    if (arg == "-s" || arg == "--sector")
    {
      if (i + 1 < argc)
      {
	string para = argv[++i];
	istringstream (para) >> sector;
	
	if (sector < MIN_SECTOR || sector > MAX_SECTOR)
	{
	  string msg = "Sectorvalue out of range";
	  print_usage (msg);
	  return false;
	}
	
	valid_sector = true;
      }
    }

    /* check for create option */
    if (arg == "-c" || arg == "--create")
    {
      opt_create = true;
    }

    /* check for directory option */
    if (arg == "-d" || arg == "--directory")
    {
      opt_dir = true;
    }
    
    /* check for dump option */
    if (arg == "-u" || arg == "--dump")
    {
      opt_dump = true;
      require_ts = true;
    }
    
    /* check for filenameparameter */
    if (arg == "-f" || arg == "--filename")
    {
      if (i + 1 < argc)
      {
	imagefilename = argv[++i];
	valid_filename = true;
      }
    }
    
  }

  if (!valid_filename)
  {
    string msg = "missing filename";
    print_usage (msg);
    return false;
  }
  
  if (require_ts && (!valid_track || !valid_sector))
  {
    string msg = "track or sector parameter required";
    print_usage (msg);
    return false;
  }
  
  cdim::cdim discImage;
  discImage.setFilename (imagefilename);
  
  if (opt_create)
  {
    return true;
  }

  if (discImage.openImage ())
  {
    /*************************
     * display directory
     *************************/
    if (opt_dir)
    {
      list <cdim::s_direntry> directory;
      discImage.getDirectory (directory);
      
      if (discImage.getDirectory (directory))
      {
	cout << "discname: " << discImage.getDiscname () << " " << discImage.getDiscID () << " " << discImage.getDosType () << endl;
	
	list <cdim::s_direntry>::iterator directory_it;
	directory_it = directory.begin ();

	while (directory_it != directory.end () )
	{
	  cdim::s_direntry entry = *directory_it;
	  cout << entry.filesize << "\t\"" << entry.filename << "\" ";
	  	      
	  if (entry.file_locked)
	  {
	    cout << "<";
	  }
	  else
	  {
	    cout << " ";
	  }

	  string filetype = "";
	  
	  switch (entry.filetype)
	  {
	    case cdim::e_ft_DEL:
	      filetype = "DEL";
	      break;
	    case cdim::e_ft_SEQ:
	      filetype = "SEQ";
	      break;
	    case cdim::e_ft_PRG:
	      filetype = "PRG";
	      break;
	    case cdim::e_ft_USR:
	      filetype = "USR";
	      break;
	    case cdim::e_ft_REL:
	      filetype = "REL";
	      break;
	    default:
	      filetype = "Invalid";
	      break;
	  }

	  cout << filetype << endl;
	  directory_it++;
	}
      }
      else
      {
	cerr << "couldn't read directoryfrom image " << imagefilename << endl;
	return false;
      }      
    }

    /*************************
     * dump a block
     *************************/
    if (opt_dump)
    {
      vector <unsigned char> dump_content;
      vector <unsigned char>::iterator it_dump_content;
      
      if (discImage.readSector (track, sector, dump_content))
      {
	unsigned char c[16];
	int i,j;
	
	it_dump_content = dump_content.begin ();
	
	while (it_dump_content != dump_content.end ())
	{
	  for (i = 0; i < 16 && it_dump_content != dump_content.end (); i++)
	  {
	    c[i] = *it_dump_content;
	    it_dump_content++;
	  }
	    
	  for (j = 0; j < i; j++)
	  {
	    cout << setw(2) << setfill('0') << hex << (int) c[j] << " ";
	  }
	    
	  cout << "\t";
	    
	  for (j = 0; j < i; j++)
	  {
	    if (isprint(c[j]))
	    {
	      cout << c[j];
	    }
	    else
	    {
	      cout << ".";
	    }
	  }	    
	  cout << endl;
	}
      }
      else
      {
	cerr << "failed to dump Track/Sector " << track << "/" << sector << endl;
      }
    }
  }
  else
  {
    cerr << "failed to open imagefile:" << imagefilename << endl;
    return false;
  }
 
  /*cdim::cdim disc1;
	
	disc1.setFilename ("test.d64");
	
	if (disc1.openImage ())
	{
	  list <cdim::s_direntry> thedir;
	  
	  int i = 0;
	  disc1.getDirectory (thedir);
	  if (disc1.getDirectory (thedir))
	  {
	    list <cdim::s_direntry>::iterator thedir_it;
	    thedir_it = thedir.begin ();

	    cout << "Anzahl: " << thedir.size () << endl;
	    
	    while (thedir_it != thedir.end () )
	    {
	      cdim::s_direntry entry = *thedir_it;
	      cout << i << " \"" << entry.filename << "\"";
	      
	      if (entry.file_locked)
	      {
		cout << "<";
	      }
	      
	      if (entry.file_open)
	      {
		cout << "   open";
	      }
	      
	      cout << "|" << entry.rel_sidetrack << "|" << endl;
	      
	      i++;
	      thedir_it++;
	    }
	  }
	  else
	  {
	    cerr << "couldn't read directory" << endl;
	  }
	}
	else
	{
	  cerr << "failed to open image" << endl;
	}
	
	if (disc1.extractFileByIndex (1, "testm2.bin", cdim::e_PRG_strip_linker))
	{
	  cout << "extracted" << endl;
	}
	else
	{
	  cout << "failed to extract" << endl;
	}

	if (disc1.extractFileByName ("MDR-DEPACKER.INS", "testm3.bin", cdim::e_PRG_strip_linker))
	{
	  cout << "2.extracted" << endl;
	}
	else
	{
	  cout << "2.failed to extract" << endl;
	}
*/
/*
	if (!disc1.openImage ("test.d64"))
	{
	  cout << "failed to open image" << endl;
	}
	
	vector <unsigned char> blubb;
	vector <unsigned char>::iterator sectorcontent_it;
	unsigned int track, sector;
	
	track = 18;
	sector = 1;
	
	if (disc1.getSector (track, sector, blubb))
	{
	  cout << "test:  " << blubb.size () << endl;
	  cout << "test2: " << blubb.capacity () << endl;
	  
	  sectorcontent_it = blubb.begin ();
	  
	  register int i, j;
	  int count = 0;
	  unsigned char c[16];
	  //int i = 0;
	  
	  while (sectorcontent_it != blubb.end ())
	  {
	    for (i = 0; i < 16 && sectorcontent_it != blubb.end (); i++)
	    {
	      c[i] = *sectorcontent_it;
	      sectorcontent_it++;
	    }
	    
	    for (j = 0; j < i; j++)
	    {
	      cout << setw(2) << setfill('0') << hex << (int) c[j] << " ";
	    }
	    
	    cout << "\t";
	    
	    for (j = 0; j < i; j++)
	    {
	      if (isprint(c[j]))
	      {
		cout << c[j];
	      }
	      else
	      {
		cout << ".";
	      }
	    }	    
	    cout << endl;
	  }
	  cout << endl;
	}
	else
	{
	  cout << "failed" << endl;
	}
	
	disc1.closeImage ();
	/*
	
	if (imgfile.is_open ())
	{
	  track=18;
	  sector=1;
	  
	  if (disc1.getSector (imgfile, track, sector, sectorcontent))
	  {
	    cout << "size: " << sectorcontent.size () << endl;
	    cout << "startpos: " << track << endl;
	    cout << "endpos: " << sector << endl << endl;
	    cout << "proof: " << setw(2) << setfill('0') << hex << (int) sectorcontent[0] << " " << hex << (int) sectorcontent[1] << endl;
	    	    
	    sectorcontent_it = sectorcontent.begin ();
	  
	    int i = 0;
	  
	    while (sectorcontent_it != sectorcontent.end ())
	    {
	      unsigned char a;
	      a=*sectorcontent_it;
	      cout << setw(2) << setfill('0') << hex << (int)a << " ";
	      if (i >= 39) { cout << endl; i=0; }
	      sectorcontent_it++;
	      i++;
	    }
	  }
	  else
	  {
	    cout << "failed" << endl;
	  }
	}
	else
	{
	  cout << "fehler:" << strerror(errno) << endl;
	}
	*/

	return 0;
}

