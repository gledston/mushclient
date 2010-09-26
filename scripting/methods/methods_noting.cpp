// methods_noting.cpp

// Stuff for writing to the output window (notes, colour notes etc.)

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\errors.h"


// world.note - makes a comment in the output buffer (with newline)

void CMUSHclientDoc::Note(LPCTSTR Message) 
{
  CString strMsg = Message;

  if (strMsg.Right (2) != ENDLINE)
    strMsg += ENDLINE;      // add a new line if necessary

  Tell (strMsg);
}   // end of CMUSHclientDoc::Note

// world.tell - makes a comment in the output buffer (without newline)

void CMUSHclientDoc::Tell(LPCTSTR Message) 
{

  // don't muck around if empty message
  if (Message [0] == 0)
    return;

  // if output buffer doesn't exist yet, remember note for later
  if (m_pCurrentLine == NULL)
    {
    COLORREF fore = m_iNoteColourFore, 
             back = m_iNoteColourBack;

    // need to do this in case a normal Note follows a ColourNote ...

    // select correct colour, if needed, from custom pallette
    if (!m_bNotesInRGB)
      {
      if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
        {   
        fore = m_customtext [m_iNoteTextColour];
        back = m_customback [m_iNoteTextColour];
        }  // end of notes in custom colour
      }

    m_OutstandingLines.push_back (CPaneStyle (Message, fore, back, m_iNoteStyle));
    return;
    }

  // If current line is not a note line, force a line change (by displaying
  // an empty string), so that the style change is on the note line and not
  // the back of the previous line. This has various implications, including
  // the way that "bleed colour to edge" will work. Otherwise it bleeds the
  // forthcoming note colour to the edge of the previous (mud) line.

  if ((m_pCurrentLine->flags & NOTE_OR_COMMAND) != COMMENT)
    DisplayMsg ("", 0, COMMENT);

CStyle * pOldStyle = NULL;

  if (!m_pCurrentLine->styleList.IsEmpty ())
    pOldStyle = m_pCurrentLine->styleList.GetTail ();

  if (m_bNotesInRGB)
    {
    // change style if we need to
    if (!(pOldStyle &&
        (pOldStyle->iFlags & COLOURTYPE) == COLOUR_RGB &&
        pOldStyle->iForeColour == m_iNoteColourFore &&
        pOldStyle->iBackColour == m_iNoteColourBack &&
        (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
        ))
        AddStyle (COLOUR_RGB | m_iNoteStyle, 
                  m_iNoteColourFore, m_iNoteColourBack, 0, NULL);
    }   // end or RGB notes
  else
    {
    // finally found Poremenos's bug - he was doing a world.colournote
    // with the note colour as  -1 (65535) which gave an access violation
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      // change style if we need to
      if (!(pOldStyle &&
          (pOldStyle->iFlags & COLOURTYPE) == COLOUR_ANSI &&
          pOldStyle->iForeColour == WHITE &&
          pOldStyle->iBackColour == BLACK &&
          (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
          ))
          AddStyle (COLOUR_ANSI | m_iNoteStyle, 
                    WHITE, BLACK, 0, NULL);
      }
    else
      {
      // change style if we need to
      if (!(pOldStyle &&
          (pOldStyle->iFlags & COLOURTYPE) == COLOUR_CUSTOM &&
          pOldStyle->iForeColour == m_iNoteTextColour &&
          pOldStyle->iBackColour == BLACK &&
          (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
          ))
          AddStyle (COLOUR_CUSTOM | m_iNoteStyle, 
                    m_iNoteTextColour, BLACK, 0, NULL);
      } // not samecolour
    } // end of palette notes

  DisplayMsg (Message, strlen (Message), COMMENT);
} // end of CMUSHclientDoc::Tell


short CMUSHclientDoc::GetNoteColour() 
{
  if (m_bNotesInRGB)
    return -1;

	return m_iNoteTextColour == SAMECOLOUR ? 0 : m_iNoteTextColour + 1;
}   // end of CMUSHclientDoc::GetNoteColour

void CMUSHclientDoc::SetNoteColour(short nNewValue) 
{
  if (nNewValue >= 0 && nNewValue <= MAX_CUSTOM)
    {
    m_iNoteTextColour = nNewValue - 1;
    m_bNotesInRGB = false;
    }
}    // end of CMUSHclientDoc::SetNoteColour

long CMUSHclientDoc::GetNoteColourFore() 
{
  if (m_bNotesInRGB)
    return m_iNoteColourFore;

  if (m_iNoteTextColour == SAMECOLOUR)
    {
    if (m_bCustom16isDefaultColour)
      return m_customtext [15];
    else
      return m_normalcolour [WHITE];
    } // not same colour
  else
  	return m_customtext [m_iNoteTextColour];
}   // end of CMUSHclientDoc::GetNoteColourFore

void CMUSHclientDoc::SetNoteColourFore(long nNewValue) 
{
// convert background to RGB if necessary

  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        m_iNoteColourBack = m_customback [15];
      else
        m_iNoteColourBack = m_normalcolour [BLACK];
      } // not same colour
    else
  	  m_iNoteColourBack = m_customback [m_iNoteTextColour];
    }  // end of not notes in RGB

  m_bNotesInRGB = true;
  m_iNoteColourFore = nNewValue & 0x00FFFFFF;

}   // end of CMUSHclientDoc::SetNoteColourFore

long CMUSHclientDoc::GetNoteColourBack() 
{
  if (m_bNotesInRGB)
    return m_iNoteColourBack;

  if (m_iNoteTextColour == SAMECOLOUR)
    {
    if (m_bCustom16isDefaultColour)
      return m_customback [15];
    else
      return m_normalcolour [BLACK];
    } // not same colour
  else
  	return m_customback [m_iNoteTextColour];
}   // end of CMUSHclientDoc::GetNoteColourBack

void CMUSHclientDoc::SetNoteColourBack(long nNewValue) 
{
// convert foreground to RGB if necessary
  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        m_iNoteColourFore = m_customtext [15];
      else
        m_iNoteColourFore = m_normalcolour [WHITE];
      } // not same colour
    else
  	  m_iNoteColourFore = m_customtext [m_iNoteTextColour];
    }  // end of not notes in RGB

  m_bNotesInRGB = true;
  m_iNoteColourBack = nNewValue & 0x00FFFFFF;

}    // end of CMUSHclientDoc::SetNoteColourBack

void CMUSHclientDoc::NoteColourRGB(long Foreground, long Background) 
{
  m_bNotesInRGB = true;
  m_iNoteColourFore = Foreground & 0x00FFFFFF;
  m_iNoteColourBack = Background & 0x00FFFFFF;
}   // end of CMUSHclientDoc::NoteColourRGB

void CMUSHclientDoc::NoteColourName(LPCTSTR Foreground, LPCTSTR Background) 
{
// convert background to RGB if necessary
  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        {
        m_iNoteColourFore = m_customtext [15];
        m_iNoteColourBack = m_customback [15];
        }
      else
        {
        m_iNoteColourFore = m_normalcolour [WHITE];
        m_iNoteColourBack = m_normalcolour [BLACK];
        }
      }   // end of notes were "same colour" (as what?)
    else
      // just normal custom colours - however check in range just in case
      if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
        {   // notes were a custom colour
        m_iNoteColourFore = m_customtext [m_iNoteTextColour];
        m_iNoteColourBack = m_customback [m_iNoteTextColour];
        }  // end of notes in custom colour
      else
        {   // white on black - shouldn't get here
        m_iNoteColourFore = RGB (255, 255, 255);
        m_iNoteColourBack = RGB (0, 0, 0);
        }  // end of notes in custom colour

    }   // end of not notes in RGB already

  m_bNotesInRGB = true;
  SetColour (Foreground, m_iNoteColourFore);
  SetColour (Background, m_iNoteColourBack);
}   // end of CMUSHclientDoc::NoteColourName


void CMUSHclientDoc::AnsiNote(LPCTSTR Text) 
{
// save old colours
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;
unsigned short iOldNoteStyle = m_iNoteStyle;

bool bBold = false;
bool bInverse = false;
bool bItalic = false;
bool bUnderline = false;
int iCurrentForeGround = WHITE;
int iCurrentBackGround = BLACK;

m_iNoteStyle = NORMAL;   // start off with normal style

const char * p,
           * start;
char c;
long length;

  p = start = Text;
  while (c = *p)
    {
    if (c == ESC)
      {
      length = p - start;

      // output earlier block
      if (length > 0)
        Tell (CString (start, length));
      p++;    // skip the ESC

      if (*p == '[')
        {
        p++;    // skip the [

        int iCode = 0;
        while (isdigit (*p) || *p == ';' || *p == 'm')
          {
          if (isdigit (c = *p))
            {
            iCode *= 10;
            iCode += c - '0';
            }
          else
            if (c == ';' || c == 'm')
              {
              switch (iCode)
                {
                // reset colours to defaults
                case ANSI_RESET:
                   iCurrentForeGround = WHITE;
                   iCurrentBackGround = BLACK;
                   bBold = false;     
                   bInverse = false;
                   bItalic = false;   
                   bUnderline = false;
                   break;

                // bold
                case ANSI_BOLD:
                   bBold = true;
                   break;

                // inverse
                case ANSI_INVERSE:
                   bInverse = true;
                   break;

                // blink
                case ANSI_BLINK:
                case ANSI_SLOW_BLINK:
                case ANSI_FAST_BLINK:
                   bItalic = true;
                   break;

                // underline
                case ANSI_UNDERLINE:
                   bUnderline = true;
                   break;

                // not bold
                case ANSI_CANCEL_BOLD:
                   bBold = false;
                   break;

                // not inverse
                case ANSI_CANCEL_INVERSE:
                   bInverse = false;
                   break;

                // not blink
                case ANSI_CANCEL_BLINK:
                case ANSI_CANCEL_SLOW_BLINK:
                   bItalic = false;
                   break;

                // not underline
                case ANSI_CANCEL_UNDERLINE:
                   bUnderline = false;
                   break;

                // different foreground colour
                case ANSI_TEXT_BLACK:
                case ANSI_TEXT_RED    :
                case ANSI_TEXT_GREEN  :
                case ANSI_TEXT_YELLOW :
                case ANSI_TEXT_BLUE   :
                case ANSI_TEXT_MAGENTA:
                case ANSI_TEXT_CYAN   :
                case ANSI_TEXT_WHITE  :
                   iCurrentForeGround = iCode - ANSI_TEXT_BLACK;
                   break;

                // different background colour
                case ANSI_BACK_BLACK  :
                case ANSI_BACK_RED    :
                case ANSI_BACK_GREEN  :
                case ANSI_BACK_YELLOW :
                case ANSI_BACK_BLUE   :
                case ANSI_BACK_MAGENTA:
                case ANSI_BACK_CYAN   :
                case ANSI_BACK_WHITE  :
                   iCurrentBackGround = iCode - ANSI_BACK_BLACK;
                   break;

                } // end of switch

              m_iNoteStyle = NORMAL;

              // select colours
              if (bBold)
                {
                SetNoteColourFore (m_boldcolour [iCurrentForeGround]);
                SetNoteColourBack (m_normalcolour [iCurrentBackGround]);
                m_iNoteStyle |= HILITE;
                }
              else
                {
                SetNoteColourFore (m_normalcolour [iCurrentForeGround]);
                SetNoteColourBack (m_normalcolour [iCurrentBackGround]);
                }

              // select other style bits
              if (bInverse)
                m_iNoteStyle |= INVERSE;

              if (bItalic)
                m_iNoteStyle |= BLINK;

              if (bUnderline)
                m_iNoteStyle |= UNDERLINE;

              p++;  // skip m or ;
              }   // end of ESC [ nn ; or ESC [ nn m

          if (c == ';')
            iCode = 0;
          else
            if (c == 'm')
              break;
            else
              p++;    // next character
          } // end of getting code
        } // end of ESC [ something
       else
         p++; // skip it

      start = p;  // ready to start a new batch
      } // end of ESC something
    else
      p++;  // just keep counting characters

    } // end of processing each character

// output remaining text  - and newline
Note (start);

// put the colours back
if (bOldNotesInRGB)
  {
  m_iNoteColourFore = iOldNoteColourFore;
  m_iNoteColourBack = iOldNoteColourBack;
  }
else  
  m_bNotesInRGB = false;

// put style back
m_iNoteStyle = iOldNoteStyle;

} // end of CMUSHclientDoc::AnsiNote

void CMUSHclientDoc::ColourNote(LPCTSTR TextColour, 
                                LPCTSTR BackgroundColour, 
                                LPCTSTR Text) 
{
  CString strMsg = Text;

  if (strMsg.Right (2) != ENDLINE)
    strMsg += ENDLINE;      // add a new line if necessary

  ColourTell (TextColour, BackgroundColour, strMsg);

}  // end of CMUSHclientDoc::ColourNote

void CMUSHclientDoc::ColourTell(LPCTSTR TextColour, LPCTSTR BackgroundColour, LPCTSTR Text) 
{

// save old colours
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;

// change colours
  NoteColourName(TextColour, BackgroundColour);

// do the tell
  Tell (Text);

// put the colours back
if (bOldNotesInRGB)
  {
  m_iNoteColourFore = iOldNoteColourFore;
  m_iNoteColourBack = iOldNoteColourBack;
  }
else  
  m_bNotesInRGB = false;

}   // end of CMUSHclientDoc::ColourTell


void CMUSHclientDoc::Hyperlink(LPCTSTR Action, 
                               LPCTSTR Text, 
                               LPCTSTR Hint, 
                               LPCTSTR TextColour, 
                               LPCTSTR BackColour, 
                               BOOL URL) 
{
  // return if attempt to do tell (or note) before output buffer exists
  if (m_pCurrentLine == NULL)
    return;

  // don't muck around if empty message
  if (Action [0] == 0)
    return;

  // If current line is not a note line, force a line change (by displaying
  // an empty string), so that the style change is on the note line and not
  // the back of the previous line. This has various implications, including
  // the way that "bleed colour to edge" will work. Otherwise it bleeds the
  // forthcoming note colour to the edge of the previous (mud) line.

  if ((m_pCurrentLine->flags & NOTE_OR_COMMAND) != COMMENT)
    DisplayMsg ("", 0, COMMENT);

  COLORREF forecolour = m_iHyperlinkColour,
           backcolour;

  if (m_bNotesInRGB)
    backcolour = m_iNoteColourBack;
  else
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        backcolour =  m_customback [15];
      else
        backcolour =  m_normalcolour [BLACK];
      } // not same colour
    else
  	  backcolour =  m_customback [m_iNoteTextColour];
    }

  SetColour (TextColour, forecolour);
  SetColour (BackColour, backcolour);

  // change to underlined hyperlink
  AddStyle (COLOUR_RGB | 
            (URL ? ACTION_HYPERLINK : ACTION_SEND) | 
            UNDERLINE, 
            forecolour, 
            backcolour, 0, 
            GetAction (Action, 
                        Hint [0] == 0 ? Action : Hint, 
                        ""));

  // output the link text
  if (strlen (Text) > 0)
    AddToLine (Text, 0);
  else
    AddToLine (Action, 0);

  // add another style to finish the hyperlink

  if (m_bNotesInRGB)
    AddStyle (COLOUR_RGB, m_iNoteColourFore, m_iNoteColourBack, 0, NULL);
  else
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        AddStyle (COLOUR_CUSTOM, 15, BLACK, 0, NULL);
      else
        AddStyle (COLOUR_ANSI, WHITE, BLACK, 0, NULL);
      } // not same colour
    else
      AddStyle (COLOUR_CUSTOM, m_iNoteTextColour, BLACK, 0, NULL);
    } // not RGB

}   // end of CMUSHclientDoc::Hyperlink

/*

  Notes text style:

  0 = normal
  1 = bold
  2 = underline
  4 = blink (italic)
  8 = inverse

  */

void CMUSHclientDoc::NoteStyle(short Style) 
{
  m_iNoteStyle = Style & TEXT_STYLE;    // only want those bits
}   // end of CMUSHclientDoc::NoteStyle

short CMUSHclientDoc::GetNoteStyle() 
{
	return m_iNoteStyle & TEXT_STYLE;
}   // end of CMUSHclientDoc::GetNoteStyle


// horizontal rule (HR)

void CMUSHclientDoc::NoteHr() 
{
  // wrap up previous line if necessary
  if (m_pCurrentLine->len > 0)
     StartNewLine (true, 0);

  // mark line as HR line
  m_pCurrentLine->flags = HORIZ_RULE;
  
  StartNewLine (true, 0); // now finish this line

  // refresh views

  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;

		  pmyView->addedstuff();
	    }	
    }

}  // end of CMUSHclientDoc::NoteHr
