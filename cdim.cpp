#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <iterator>
#include <iomanip>	// for setw/setfill
#include <cerrno>
#include <cstring>
#include <locale>

#include "libcdim.h"

using namespace std;

#define MIN_TRACK 1
#define MAX_TRACK 40
#define MIN_SECTOR 0
#define MAX_SECTOR 22

string conv2utf8 (string);

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
  cout << "\t-r, --rawmode\tprint directory/filenames in raw screencode" << endl;
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
  
  bool rawmode, valid_track, valid_sector, require_ts;
  rawmode = false;
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
    
    /* check for rawmode */
    if (arg == "-r" || arg == "--rawmode")
    {
      rawmode = true;
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
	string discname, discid, dostype, filename;
	
	if (rawmode)
	{
	  discname = discImage.getDiscname ();
	  discid = discImage.getDiscID ();
	  dostype = discImage.getDosType ();
	}
	else
	{
	  discname = conv2utf8 (discImage.getDiscname ());
	  discid = conv2utf8 (discImage.getDiscID ());
	  dostype = conv2utf8 (discImage.getDosType ());
	}
	
	cout << "0 \"" << discname << "\" " << discid << " " << dostype << endl;
	
	list <cdim::s_direntry>::iterator directory_it;
	directory_it = directory.begin ();

	while (directory_it != directory.end () )
	{
	  cdim::s_direntry entry = *directory_it;
	  cout << setw(5) << left << entry.filesize << "\"";
	  
	  if (rawmode)
	  {
	    filename = entry.filename + "\"";
	  }
	  else
	  {
	    filename = conv2utf8 (entry.filename) + "\"";
	  }
	  
	  cout << setw (17) << left << filename << " ";
	  
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
	      filetype = "?";
	      break;
	  }

	  cout << filetype;
	  
  	  if (entry.file_locked)
	  {
	    cout << "<";
	  }
	  
	  cout << endl;
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

  return 0;
}

string conv2utf8 (string convstr)
{
  string petascii2utf [] =
  {
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uF100",       //WHITE
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uF118",       //DISABLE
    "\uF119",       //ENABLE
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\u000D",       //CARRIAGE
    "\u000E",       //SHIFT
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uF11C",       //CURSOR
    "\uF11A",       //REVERSE
    "\uF120",       //HOME
    "\u007F",       //DELETE
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uF101",       //RED
    "\uF11D",       //CURSOR
    "\uF102",       //GREEN
    "\uF103",       //BLUE
    "\u0020",       //SPACE
    "\u0021",       //EXCLAMATION
    "\u0022",       //QUOTATION
    "\u0023",       //NUMBER
    "\u0024",       //DOLLAR
    "\u0025",       //PERCENT
    "\u0026",       //AMPERSAND
    "\u0027",       //APOSTROPHE
    "\u0028",       //LEFT
    "\u0029",       //RIGHT
    "\u002A",       //ASTERISK
    "\u002B",       //PLUS
    "\u002C",       //COMMA
    "\u002D",       //HYPHEN-MINUS
    "\u002E",       //FULL
    "\u002F",       //SOLIDUS
    "\u0030",       //DIGIT
    "\u0031",       //DIGIT
    "\u0032",       //DIGIT
    "\u0033",       //DIGIT
    "\u0034",       //DIGIT
    "\u0035",       //DIGIT
    "\u0036",       //DIGIT
    "\u0037",       //DIGIT
    "\u0038",       //DIGIT
    "\u0039",       //DIGIT
    "\u003A",       //COLON
    "\u003B",       //SEMICOLON
    "\u003C",       //LESS-THAN
    "\u003D",       //EQUALS
    "\u003E",       //GREATER-THAN
    "\u003F",       //QUESTION
    "\u0040",       //COMMERCIAL
    "\u0041",       //LATIN
    "\u0042",       //LATIN
    "\u0043",       //LATIN
    "\u0044",       //LATIN
    "\u0045",       //LATIN
    "\u0046",       //LATIN
    "\u0047",       //LATIN
    "\u0048",       //LATIN
    "\u0049",       //LATIN
    "\u004A",       //LATIN
    "\u004B",       //LATIN
    "\u004C",       //LATIN
    "\u004D",       //LATIN
    "\u004E",       //LATIN
    "\u004F",       //LATIN
    "\u0050",       //LATIN
    "\u0051",       //LATIN
    "\u0052",       //LATIN
    "\u0053",       //LATIN
    "\u0054",       //LATIN
    "\u0055",       //LATIN
    "\u0056",       //LATIN
    "\u0057",       //LATIN
    "\u0058",       //LATIN
    "\u0059",       //LATIN
    "\u005A",       //LATIN
    "\u005B",       //LEFT
    "\u00A3",       //POUND
    "\u005D",       //RIGHT
    "\u2191",       //UPWARDS
    "\u2190",       //LEFTWARDS
    "\u2501",       //BOX
    "\u2660",       //BLACK
    "\u2502",       //BOX
    "\u2501",       //BOX
    "\uF122",       //BOX
    "\uF123",       //BOX
    "\uF124",       //BOX
    "\uF126",       //BOX
    "\uF128",       //BOX
    "\u256E",       //BOX
    "\u2570",       //BOX
    "\u256F",       //BOX
    "\uF12A",       //ONE
    "\u2572",       //BOX
    "\u2571",       //BOX
    "\uF12B",       //ONE
    "\uF12C",       //ONE
    "\u25CF",       //BLACK
    "\uF125",       //BOX
    "\u2665",       //BLACK
    "\uF127",       //BOX
    "\u256D",       //BOX
    "\u2573",       //BOX
    "\u25CB",       //WHITE
    "\u2663",       //BLACK
    "\uF129",       //BOX
    "\u2666",       //BLACK
    "\u253C",       //BOX
    "\uF12E",       //LEFT
    "\u2502",       //BOX
    "\u03C0",       //GREEK
    "\u25E5",       //BLACK
    "\uFFEF",       //UNDEFINED
    "\uF104",       //ORANGE
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uFFEF",       //UNDEFINED
    "\uF110",       //FUNCTION
    "\uF112",       //FUNCTION
    "\uF114",       //FUNCTION
    "\uF116",       //FUNCTION
    "\uF111",       //FUNCTION
    "\uF113",       //FUNCTION
    "\uF115",       //FUNCTION
    "\uF117",       //FUNCTION
    "\u000A",       //LINE
    "\u000F",       //SHIFT
    "\uFFEF",       //UNDEFINED
    "\uF105",       //BLACK
    "\uF11E",       //CURSOR
    "\uF11B",       //REVERSE
    "\u000C",       //FORM
    "\uF121",       //INSERT
    "\uF106",       //BROWN
    "\uF107",       //LIGHT
    "\uF108",       //GRAY
    "\uF109",       //GRAY
    "\uF10A",       //LIGHT
    "\uF10B",       //LIGHT
    "\uF10C",       //GRAY
    "\uF10D",       //PURPLE
    "\uF11D",       //CURSOR
    "\uF10E",       //YELLOW
    "\uF10F",       //CYAN
    "\u00A0",       //NO-BREAK
    "\u258C",       //LEFT
    "\u2584",       //LOWER
    "\u2594",       //UPPER
    "\u2581",       //LOWER
    "\u258F",       //LEFT
    "\u2592",       //MEDIUM
    "\u2595",       //RIGHT
    "\uF12F",       //LOWER
    "\u25E4",       //BLACK
    "\uF130",       //RIGHT
    "\u251C",       //BOX
    "\uF134",       //BLACK
    "\u2514",       //BOX
    "\u2510",       //BOX
    "\u2582",       //LOWER
    "\u250C",       //BOX
    "\u2534",       //BOX
    "\u252C",       //BOX
    "\u2524",       //BOX
    "\u258E",       //LEFT
    "\u258D",       //LEFT
    "\uF131",       //RIGHT
    "\uF132",       //UPPER
    "\uF133",       //UPPER
    "\u2583",       //LOWER
    "\uF12D",       //ONE
    "\uF135",       //BLACK
    "\uF136",       //BLACK
    "\u2518",       //BOX
    "\uF137",       //BLACK
    "\uF138",       //TWO
    "\u2501",       //BOX
    "\u2660",       //BLACK
    "\u2502",       //BOX
    "\u2501",       //BOX
    "\uF122",       //BOX
    "\uF123",       //BOX
    "\uF124",       //BOX
    "\uF126",       //BOX
    "\uF128",       //BOX
    "\u256E",       //BOX
    "\u2570",       //BOX
    "\u256F",       //BOX
    "\uF12A",       //ONE
    "\u2572",       //BOX
    "\u2571",       //BOX
    "\uF12B",       //ONE
    "\uF12C",       //ONE
    "\u25CF",       //BLACK
    "\uF125",       //BOX
    "\u2665",       //BLACK
    "\uF127",       //BOX
    "\u256D",       //BOX
    "\u2573",       //BOX
    "\u25CB",       //WHITE
    "\u2663",       //BLACK
    "\uF129",       //BOX
    "\u2666",       //BLACK
    "\u253C",       //BOX
    "\uF12E",       //LEFT
    "\u2502",       //BOX
    "\u03C0",       //GREEK
    "\u25E5",       //BLACK
    "\u00A0",       //NO-BREAK
    "\u258C",       //LEFT
    "\u2584",       //LOWER
    "\u2594",       //UPPER
    "\u2581",       //LOWER
    "\u258F",       //LEFT
    "\u2592",       //MEDIUM
    "\u2595",       //RIGHT
    "\uF12F",       //LOWER
    "\u25E4",       //BLACK
    "\uF130",       //RIGHT
    "\u251C",       //BOX
    "\uF134",       //BLACK
    "\u2514",       //BOX
    "\u2510",       //BOX
    "\u2582",       //LOWER
    "\u250C",       //BOX
    "\u2534",       //BOX
    "\u252C",       //BOX
    "\u2524",       //BOX
    "\u258E",       //LEFT
    "\u258D",       //LEFT
    "\uF131",       //RIGHT
    "\uF132",       //UPPER
    "\uF133",       //UPPER
    "\u2583",       //LOWER
    "\uF12D",       //ONE
    "\uF135",       //BLACK
    "\uF136",       //BLACK
    "\u2518",       //BOX
    "\uF137",       //BLACK
    "\u03C0"	    //GREEK
  };
  
  string thenew = "";
  string::iterator convstr_it = convstr.begin ();
  
  while (convstr_it != convstr.end ())
  {
    thenew += petascii2utf[(unsigned char)*convstr_it];
    convstr_it++;
  }
  
  return thenew;
}
