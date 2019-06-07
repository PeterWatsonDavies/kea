// Copyright (C) 2015-2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Produce System Messages Manual
//
// This tool reads all the message files given on the command line.
// It pulls all the messages and description out, sorts them by
// message ID, and writes them out as a single (formatted) file.
//
// Invocation:
// The code is invoked using the command line:
//
// system_messages [-o <output-file>] <files>
//
// If no output file is specified, output is written to stdout.
// The produced format is docbook XML.

#include <config.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <stdlib.h>
#include <cstring>

#include <boost/lexical_cast.hpp>

typedef std::vector<std::string> LinesType;

/// @brief dictionary values
struct Details {
    std::string text;
    LinesType description;
    std::string sname;
    std::string filename;
};

/// @brief Main dictionary holding all the messages.
/// The messages are accumulated here before being printed in
/// alphabetical order.
typedef std::map<const std::string, Details> DictionaryType;
DictionaryType dictionary;

/// @brief The structure of the output page
//
///        header
///           section header
///                 message
///              separator
///                 message
///              separator
///                :
///              separator
///                 message
///           section trailer
///           separator
///           section header
///             :
///           section trailer
///        trailer
//
/// (Indentation is not relevant - it has only been added to the above
/// illustration to make the structure clearer.)  The text of these section is:

/// @name Constants for the output page
//@{

/// @brief File header
/// this is output before anything else.
const std::string FILE_HEADER =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE book [\
<!ENTITY % keaversion SYSTEM \"version.ent\">\
%keaversion;\
]>\
\n\
<!--\n\
     This XML document is generated using the system_messages tool\n\
     based on the .mes message files.\n\
\n\
     Do not edit this file.\n\
-->\n\
<book xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\">\n\
  <?xml-stylesheet href=\"kea-guide.css\" type=\"text/css\"?>\n\
\n\
  <info>\n\
    <title>Kea Messages Manual</title>\n\
\n\
    <copyright>\n\
      <year>2011-2019</year>\n\
      <holder>Internet Systems Consortium, Inc. (\"ISC\")</holder>\n\
    </copyright>\n\
\n\
    <abstract>\n\
      <para>\n\
        This is the messages manual for Kea version &keaversion;.\n\
            The most up-to-date version of this document, along with\n\
            other documents for Kea, can be found at\n\
        <ulink url=\"https://jenkins.isc.org/job/Kea_doc/messages/kea-messages.html\"/>.\n\
      </para>\n\
    </abstract>\n\
\n\
    <releaseinfo>This is the messages manual for Kea version\n\
        &keaversion;.</releaseinfo>\n\
  </info>\n\
\n\
  <chapter id=\"intro\">\n\
    <title>Introduction</title>\n\
    <para>\n\
      This document lists each message that can be logged by the\n\
      programs in the Kea package.  Each entry in this manual\n\
      is of the form:\n\
      <screen>IDENTIFICATION message-text</screen>\n\
      ... where \"IDENTIFICATION\" is the message identification included\n\
      in each message logged and \"message-text\" is the accompanying\n\
      message text.  The \"message-text\" may include placeholders of the\n\
      form \"%1\", \"%2\" etc.; these parameters are replaced by relevant\n\
      values when the message is logged.\n\
    </para>\n\
    <para>\n\
      Each entry is also accompanied by a description giving more\n\
      information about the circumstances that result in the message\n\
      being logged.\n\
    </para>\n\
    <para>\n\
      For information on configuring and using Kea logging,\n\
      refer to the <ulink url=\"kea-guide.html\">Kea Guide</ulink>.\n\
    </para>\n\
  </chapter>\n\
\n\
  <chapter id=\"messages\">\n\
    <title>Kea Log Messages</title>\n";

/// @brief Section header
/// This is output one for each module. $M substitution token is the name.
const std::string SECTION_HEADER = "  <section id=\"$M\">\n\
    <title>$M Module</title>\n\
    <para>\n\
      <variablelist>\n";

/// @brief message ID
/// This is output once for each message.  The string contains
/// substitution tokens: $I is replaced by the message identification,
/// $T by the message text, and $D by the message description.
const std::string ID_MESSAGE =
"<varlistentry id=\"$I\">\n\
<term>$I $T</term>\n\
<listitem><para>\n\
$D</para></listitem>\n\
</varlistentry>";

/// @brief Blank line
/// A description may contain blank lines intended to separate
/// paragraphs.  If so, each blank line is replaced by the following.
const std::string BLANK = "</para><para>";

/// @brief Separator
/// The separator is copied to the output verbatim after each message except
/// the last.
const std::string SEPARATOR = "";

/// @brief Section trailer
/// The trailer is copied to the output verbatim after the last message.
const std::string SECTION_TRAILER =
"      </variablelist>\n\
    </para>\n\
  </section>";

/// @brief File trailer
/// The trailer is copied to the output verbatim after the last section.
const std::string FILE_TRAILER =
"  </chapter>\n\
</book>";

//@}

/// @name Utility routines
//@{

/// @brief Report an error and exit
void reportError(const std::string& filename, const std::string& what)
{
    std::cerr << "*** ERROR in " << filename << "\n";
    std::cerr << "*** REASON: " << what << "\n";
    std::cerr << "*** System message generator terminating" << "\n";
    exit(1);
}

/// @brief Replace tag
/// Replaces the '<' and '>' in text about to be inserted into the template
/// sections above with &lt; and &gt; to avoid problems with message text
/// being interpreted as XML text.
std::string replaceTag(const std::string& src)
{
    std::string result;
    for (std::string::const_iterator it = src.begin(); it != src.end(); ++it) {
        if (*it == '<') {
            result.append("&lt;");
        } else if (*it == '>') {
            result.append("&gt;");
        } else {
           result.push_back(*it);
        }
    }
    return (result);
}

/// @brief Replace shell
/// Replace $c in a string (or with other words performs macro expansion
/// with '$' for introducing a macro followed by a character selecting
/// a specific macro.
///
/// @param src  source string
/// @param c    character selecting a macro when it follows '$'
/// @param val  value which
///
/// @return     the source string where all occurrences of $c were
///             replaced by val
std::string replaceShell(const std::string& src, char c,
                         const std::string& val)
{
    std::string result;
    bool shell = false;
    for (std::string::const_iterator it = src.begin(); it != src.end(); ++it) {
        if (shell) {
            if (*it == c) {
                result.append(val);
            } else {
               result.push_back('$');
               result.push_back(*it);
            }
            shell = false;
        } else if (*it == '$') {
            shell = true;
        } else {
            result.push_back(*it);
        }
    }
    return (result);
}

/// @brief Replace blank lines
/// Replaces blank lines in an array with the contents of the 'blank' section.
LinesType replaceBlankLines(const LinesType& lines)
{
    LinesType result;
    for (LinesType::const_iterator l = lines.begin(); l != lines.end(); ++l) {
        if (l->empty()) {
            result.push_back(BLANK);
        } else {
            result.push_back(*l);
        }
    }
    return (result);
}

//@}

/// @name Printing functions
//@{

/// @brief Print file header
void printHeader() {
    std::cout << FILE_HEADER << "\n";
}

/// @brief Print separator
void printSeparator() {
    std::cout << SEPARATOR << "\n";
}

/// @brief Print section header
void printSectionHeader(const std::string& sname)
{
    // In the section name, replace "<" and ">" with XML-safe versions and
    // substitute into the data.
    std::cout << replaceShell(SECTION_HEADER, 'M', replaceTag(sname));
}

/// @brief print message id
void printMessage(const std::string& msgid)
{
    // In the message ID, replace "<" and ">" with XML-safe versions and
    // substitute into the data.
    const std::string m0 = ID_MESSAGE;
    const std::string m1 = replaceShell(m0, 'I', replaceTag(msgid));

    // Do the same for the message text.
    std::string m2 = replaceShell(m1, 'T',
                                  replaceTag(dictionary[msgid].text));

    // Do the same for the description then replace blank lines with the
    // specified separator.  (We do this in that order to avoid replacing
    // the "<" and ">" in the XML tags in the separator.)
    LinesType desc0 = dictionary[msgid].description;
    LinesType desc1;
    for (LinesType::iterator l = desc0.begin(); l != desc0.end(); ++l) {
        desc1.push_back(replaceTag(*l));
    }
    LinesType desc2 = replaceBlankLines(desc1);

    // Join the lines together to form a single string and insert into
    // current text.
    std::string m3;
    for (LinesType::iterator l = desc2.begin(); l != desc2.end(); ++l) {
        m3.append(*l);
        m3.push_back('\n');
    }

    std::cout << replaceShell(m2, 'D', m3) << "\n";
}

/// @brief print section trailer
void printSectionTrailer() {
    std::cout << SECTION_TRAILER << "\n";
}

/// @brief print file trailer
void printTrailer() {
    std::cout << FILE_TRAILER << "\n";
}

//@}

/// @brief Removes leading and trailing empty lines.
///
/// A list of strings is passed as argument, some of which may be empty.
/// This function removes from the start and end of list a contiguous
/// sequence of empty lines and returns the result.  Embedded sequence of
/// empty lines are not touched.
///
/// @param lines List of strings to be modified.
///
/// @return Input list of strings with leading/trailing blank line
/// sequences removed.
LinesType removeEmptyLeadingTrailing(const LinesType& lines)
{
    LinesType retlines = lines;

    // Dispose of degenerate case of empty array
    if (retlines.empty()) {
        return (retlines);
    }

    // Search for first non-blank line
    for (;;) {
        LinesType::iterator start = retlines.begin();
        if (start == retlines.end()) {
            return (retlines);
        }
        if (start->empty()) {
            retlines.erase(start);
        } else {
            break;
        }
    }

    // Search for last non-blank line
    for (;;) {
        LinesType::reverse_iterator finish = retlines.rbegin();
        if (finish == retlines.rend()) {
            return (retlines);
        }
        if (finish->empty()) {
            retlines.erase(retlines.end() - 1);
        } else {
            break;
        }
    }

    return (retlines);
}


/// @brief Add the current message ID and associated information to the global
/// dictionary.
/// If a message with that ID already exists, loop appending suffixes
/// of the form "(n)" to it until one is found that doesn't.
///
///  @param msgid        Message ID
///  @param msgtext      Message text
///  @param desc         Message description
///  @param filename     File from which the message came.  Currently this is
///                      not used, but a future enhancement may wish to
///                      include the name of the message file in the
///                      messages manual.
void addToDictionary(const std::string& msgid,
                     const std::string& msgtext,
                     const LinesType& desc,
                     const std::string& filename)
{
    // If the ID is in the dictionary, append a "(n)" to the name - this will
    // flag that there are multiple instances.  (However, this is an error -
    // each ID should be unique in the code.)
    std::string key = msgid;
    if (dictionary.count(key) > 0) {
        int i = 1;
        std::string s = boost::lexical_cast<std::string>(i);
        key = msgid + " (" + s + ")";
        while (dictionary.count(key) > 0) {
            i = i + 1;
            s = boost::lexical_cast<std::string>(i);
            key = msgid + " (" + s + ")";
        }
    }

    // Remove leading and trailing blank lines in the description, then
    // add everything into a subdictionary which is then added to the main
    // one.
    Details details;
    details.text = msgtext;
    details.description = removeEmptyLeadingTrailing(desc);
    size_t underscore = msgid.find_first_of('_');
    details.sname = msgid.substr(0, underscore);
    details.filename = filename;
    dictionary.insert(std::pair<const std::string, Details>(key, details));
}


/// @brief Processes file content.
/// Messages and descriptions are identified and added to a dictionary
/// (keyed by message ID).  If the key already exists, a numeric
/// suffix is added to it.

/// The format of .mes files is fully described in src/lib/log/logging.dox
///
/// @param filename     Name of the message file being processed
/// @param lines        Lines read from the file
void processFileContent(const std::string& filename,
                        const LinesType& lines)
{
    std::string prefix;         // Last prefix encountered
    std::string msgid;          // Last message ID encountered
    std::string msgtext;        // Text of the message
    LinesType description;      // Description

    for (LinesType::const_iterator l = lines.begin(); l != lines.end(); ++l) {
        if (l->empty()) {
            description.push_back(*l);
        } else if (l->at(0) == '$') {
            // Starts with "$".  Ignore anything other than $PREFIX
            char* line = new char [l->size() + 1];
            std::strcpy(line, l->c_str());
            char* word0 = strtok(line, " \t\r\n\t\v");
            if (strcasecmp(word0, "$PREFIX") == 0) {
                char* word1 = strtok(NULL, " \t\r\n\t\v");
                prefix = word1;
            }
            delete[] line;
        } else if (l->at(0) == '%') {
            // Start of a message.  Add the message we were processing to the
            // dictionary and clear everything apart from the file name.
            if (!msgid.empty()) {
                addToDictionary(msgid, msgtext, description, filename);
            }
            msgid.clear();
            msgtext.clear();
            description.clear();
            // Start of a message
            char* line = new char [l->size() + 1];
            std::strcpy(line, l->c_str());
            // Remove "%" and trim leading spaces
            size_t start = l->find_first_not_of(" \t\r\n\t\v", 1);
            if (start == std::string::npos) {
                reportError(filename, "Line with single % found");
                continue;
            }
            // Split into words.  The first word is the message ID
            char* word0 = strtok(line + start, " \t\r\n\t\v");
            msgid = prefix;
            msgid.append(word0);
            std::transform(msgid.begin(), msgid.end(),
                           msgid.begin(), toupper);
            char* word1 = strtok(NULL, " \t\r\n\t\v");
            start = word1 - line;
            size_t finish = l->find_last_not_of(" \t\r\n\t\v");
            msgtext = l->substr(start, finish + 1 - start);
        } else {
            // Part of a description, so add to the current description array
            description.push_back(*l);
        }
    }

    // All done, add the last message to the global dictionary.
    if (!msgid.empty()) {
        addToDictionary(msgid, msgtext, description, filename);
    }
}

/// @brief Process a file
/// Read it in and strip out all comments and and directives.  Leading
/// and trailing blank lines in the file are removed and the remainder
/// passed for message processing.
///
/// @param filename     Name of the message file to process
void processFile(const std::string& filename)
{
    std::ifstream cin;
    cin.open(filename.c_str(), std::ios::in);
    if (!cin.is_open()) {
        reportError(filename, "open for read failure");
    }
    LinesType lines0;
    while (!cin.eof()) {
        std::string line;
        getline(cin, line);
        lines0.push_back(line);
    }
    cin.close();

    // Trim leading and trailing spaces from each line, and remove comments.
    LinesType lines1;
    for (LinesType::iterator l = lines0.begin(); l != lines0.end(); ++l) {
        std::string line = *l;
	// Empty lines have no spaces so are processed
        if (line.empty()) {
            lines1.push_back(line);
            continue;
        }
	// Trim leading spaces
        size_t start = line.find_first_not_of(" \t\r\n\t\v");
        if (start != 0) {
            line.erase(0, start);
        }
	// Done?
        if (line.empty()) {
            lines1.push_back(line);
            continue;
        }
	// Trim trailing spaces
        size_t finish = line.find_last_not_of(" \t\r\n\t\v");
        if ((finish != std::string::npos) &&
            (finish + 1 != line.size())) {
            line.erase(finish + 1);
        }
	// Done
        if (line.empty()) {
            lines1.push_back(line);
            continue;
        }
	// Skip comments
        if (line[0] != '#') {
            lines1.push_back(line);
        }
    }

    // Remove leading/trailing empty line sequences from the result
    LinesType lines2 = removeEmptyLeadingTrailing(lines1);

    // Interpret content
    processFileContent(filename, lines2);
}

/// @brief Usage error routine
void usage(char* progname)
{
    std::cerr << "Usage: " << progname <<
        " [--help | options] files\n";
    std::cerr << " options: --output file: " <<
        "output file name (default to stdout)\n";
}

/// @brief Main (entry point)
int main(int argc, char* argv[])
{
    char* progname = argv[0];
    std::ofstream fout;
    while (argc > 1) {
        --argc;
        ++argv;
        if (strcmp(argv[0], "--help") == 0) {
             usage(progname);
             exit(0);
        }
        // Redirect output if specified (errors are written to stderr)
        if ((strcmp(argv[0], "-o") == 0) ||
            (strcmp(argv[0], "--output") == 0)) {
            --argc;
            ++argv;
            if (argc == 0) {
                usage(progname);
                exit(-1);
            }
            fout.open(argv[0], std::ofstream::out | std::ofstream::trunc);
            if (!fout.is_open()) {
                reportError(argv[0], "open for write failure");
            }
            std::cout.rdbuf(fout.rdbuf());
            --argc;
            ++argv;
            break;
        }
    }

    if (argc == 0) {
        usage(progname);
        exit(-1);
    }
    for (int i = 0; i < argc; ++i) {
        processFile(argv[i]);
    }

    // Now just print out everything we've read (in alphabetical order).
    bool first = true;
    std::string sname;
    printHeader();
    for (DictionaryType::iterator it = dictionary.begin();
         it != dictionary.end();
         ++it) {
        if (sname.compare(it->second.sname) != 0) {
            if (!sname.empty()) {
                printSectionTrailer();
                printSeparator();
            }
            sname = it->second.sname;
            printSectionHeader(sname);
            first = true;
        }
        if (!first) {
            printSeparator();
        }
        first = false;
        printMessage(it->first);
    }
    if (!sname.empty()) {
        printSectionTrailer();
    }
    printTrailer();
    exit(0);
}
