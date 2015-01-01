#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <iterator>
#include <iomanip>	// for setw/setfill
#include <cerrno>
#include <cstring>
#include "optionparser.h"

#include "libcdim.h"

using namespace std;

enum  optionIndex { UNKNOWN, HELP, DIRECTORY, DUMP, CREATE, TRACK, SECTOR };

const option::Descriptor usage[] =
{
 {UNKNOWN, 0, "", "",option::Arg::None, "USAGE: cdim [options] imagefilename\n\n"
                                        "Options:" },
 {HELP, 0,"h", "help",option::Arg::None, "  --help, -h  \tprint usage and exit" },
 {DIRECTORY, 0,"d","dir",option::Arg::None, "  --dir, -d  \tshow drectory" },
 {CREATE, 0, "c", "create", option::Arg::None, "  --create, -c  \tcreate discImage" },
 {DUMP, 0, "u", "dump", option::Arg::None, "  --dump, -u  \tdump a block (requires -s and -t" },
 {TRACK, 0, "t", "track", 1, "  --track, -t  \ttrack parameter (f.e. for --dump" },
 {SECTOR, 0, "s", "sector", option::Arg::Required, "  --sector, -s  \tsector parameter (f.e. for --dump" },
 {UNKNOWN, 0, "", "",option::Arg::None, "\nExamples:\n"
                               "  cdim --dir test.d64\n"
                               "  cdim --extract --as prg --save extractedfile.prg test.d64\n" },
 {0,0,0,0,0,0}
};


int main (int argc, char *argv[])
{
  string imagefilename = "";
  unsigned int track, sector;
  
  argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
  option::Stats  stats(usage, argc, argv);
  option::Option* options = new option::Option[stats.options_max];
  option::Option* buffer  = new option::Option[stats.buffer_max];
  option::Parser parse(usage, argc, argv, options, buffer);

  if (parse.error())
  {
    return false;
  }

  if (options[HELP] || argc == 0)
  {
    option::printUsage(std::cout, usage);
    return false;
  }

  if (parse.nonOptionsCount () != 1)
  {
    option::printUsage (cout, usage);
    
    if (parse.nonOptionsCount () == 0)
    {
      cout << "Error: no imagefilename" << endl;
    }
    else
    {
      cout << "Error: too many filenames" << endl;
    }
    
    return false;
  }
  else
  {
    imagefilename = parse.nonOption (0);
    /* TODO: link check etc. */
  }
  
  cdim::cdim discImage;
  discImage.setFilename (imagefilename);
  
  if (options[CREATE])
  {
    return true;
  }

  if (discImage.openImage ())
  {
    /*************************
     * display directory
     *************************/
    if (options[DIRECTORY])
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
    if (options[DUMP])
    {
      /* remove later, when -t/-s is implemented */
      unsigned int track, sector;
      track = 18;
      sector = 1;
      
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
 
  delete[] options;
  delete[] buffer;

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

