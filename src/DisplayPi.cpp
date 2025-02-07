//
#include "DisplayPi.h"

#include <memory.h>

#include <circle/types.h>
#include <circle/bcmpropertytags.h>
#include <circle/debug.h>

#include "res/button_1.h"
#include "res/coolspot.h"

DisplayPi::DisplayPi(CLogger* logger, CTimer* timer) :
   logger_(logger),
   timer_(timer),
   frame_buffer_(nullptr),
   full_resolution_(false),
   full_resolution_cached_(false),
   mutex_(TASK_LEVEL),
   added_line_(1),
   buffer_used_(0),
   nb_frame_in_queue_(0),
   sync_on_frame_(false)
{
   font_ = new CoolspotFont(logger_);

   for (int i = 0; i < FRAME_BUFFER_SIZE; i++)
   {
      frame_used_[i] = FR_FREE;
   }
   frame_used_[buffer_used_] = FR_USED;
}

DisplayPi::~DisplayPi()
{
   delete font_;
   if ( frame_buffer_ != nullptr)
   {
      delete frame_buffer_;
   }   
}

bool DisplayPi::Initialization()
{
   /*
   unsigned int screen_width = 640;
   unsigned int screen_height = 480;

   // Get display property, to compute best values
   CBcmPropertyTags Tags;
	TPropertyTagDisplayDimensions Dimensions;
	if (Tags.GetTag (PROPTAG_GET_DISPLAY_DIMENSIONS, &Dimensions, sizeof Dimensions))
	{
      screen_width  = Dimensions.nWidth;
      screen_height = Dimensions.nHeight;
   }

   // Now we have real width/height : compute best values for display, to have :
   // - good pixel ratio (4x3)
   
   float ratio = ((float)GetWidth())/((float)GetHeight());
   if ( (screen_width / screen_height ) < ratio )
   {
      //
      NbPixelWidth = static_cast<long>(screen_height * ratio);
      NbPixelHeight = screen_height ;
   }
   else
   {
      //
      NbPixelWidth = screen_width - 0;
      NbPixelHeight = static_cast<long>(screen_width / ratio);
   }   


      // Compute height to have a complete screen, without problem with scanlines
      if ( NbPixelHeight % m_Height != 0)
      {
         int NbPixelHeightComputed = (NbPixelHeight / m_Height) * m_Height;
         if ( NbPixelHeight % m_Height > (m_Height/2) )
         {
            NbPixelHeightComputed += m_Height;
         }
         NbPixelHeight = NbPixelHeightComputed;
         NbPixelWidth = static_cast<long>(NbPixelHeight*ratio);
      }

      int fXmin, fXmax, fYmin, fYmax;
      fXmin = (findMode.w - NbPixelWidth) / 2;
      fXmax = (findMode.w - NbPixelWidth) / 2 + NbPixelWidth;
      fYmin = ((long)findMode.h - NbPixelHeight) / 2;
      fYmax = ((long)findMode.h - NbPixelHeight) / 2 + NbPixelHeight;


      m_DestRectFullScreen.x = fXmin;
      m_DestRectFullScreen.y = fYmin;
      m_DestRectFullScreen.w = NbPixelWidth;
      m_DestRectFullScreen.h = NbPixelHeight;*/   
   // - 


   if ( frame_buffer_ != nullptr)
   {
      delete frame_buffer_;
   }
   frame_buffer_ = new CBcmFrameBuffer(768, 277*2, 32, 1024, 1024* FRAME_BUFFER_SIZE);

   frame_buffer_->Initialize();
   frame_buffer_->SetVirtualOffset(143, 47/2);

   return true;
}

bool DisplayPi::ListEDID()
{
   //  Display all resolution supported
	CBcmPropertyTags Tags;
	TPropertyTagEDIDBlock TagEDID;
	TagEDID.nBlockNumber = 0;
   bool tag_send = Tags.GetTag (PROPTAG_GET_EDID_BLOCK , &TagEDID, sizeof TagEDID, 4);
	if ( tag_send && TagEDID.nStatus == 0)
	{
      logger_->Write("Display", LogNotice, "EDID message : ");
		// Decodage :
      // check id
      unsigned char header[]={0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
      if ( memcmp( &TagEDID.Block[0], header, sizeof(header)) == 0)
      {
         // Manufacturer ID, product, serial number
         // week/year
         // EDID version
         logger_->Write("Display", LogNotice, "EDID version : %i.%i", TagEDID.Block[18], TagEDID.Block[19]);
		Tags.GetTag (PROPTAG_GET_EDID_BLOCK , &TagEDID, sizeof TagEDID, 4);

      }
      else
      {
         logger_->Write("Display", LogNotice, "EDID Wrong header");
         debug_hexdump (TagEDID.Block, 128, "EDID");
      }
	}   
   return true;
}


void DisplayPi::SetScanlines(int scan)
{
   
}

bool DisplayPi::AFrameIsReady()
{
   logger_->Write("Display", LogNotice, "A Frame is ready - NOT IMPLEMENTED ");
   return false;
}

void DisplayPi::Display()
{
   logger_->Write("Display", LogNotice, "Display - NOT IMPLEMENTED ");
}

void DisplayPi::Config()
{
   logger_->Write("Display", LogNotice, "Config - NOT IMPLEMENTED ");
}

const char* DisplayPi::GetInformations()
{
   logger_->Write("Display", LogNotice, "Get Information ");
   return "Display for Raspberry PI - Bare metal";
}

int DisplayPi::GetWidth()
{
   return 1024;
}

int DisplayPi::GetHeight()
{
   return 1024;
}

void DisplayPi::SetSize(IDisplay::SizeEnum size)
{
   logger_->Write("Display", LogNotice, "SetSize - NOT IMPLEMENTED ");
}

IDisplay::SizeEnum  DisplayPi::GetSize()
{
   logger_->Write("Display", LogNotice, "SetSize - NOT IMPLEMENTED ");
   return IDisplay::S_STANDARD;
}

struct TBlankScreen
{
   TPropertyTag	Tag;
   u32            blank;

}
PACKED;

void DisplayPi::Loop()
{
   logger_->Write("DIS", LogNotice, "Starting loop");
   // Waiting for a new frame to display
   //int old_frame_index = -1;
   while (1)
   {
      // Display available frame
      int frame_index = -1;
      mutex_.Acquire();
      if (nb_frame_in_queue_ > 0)
      {
         /*if ( old_frame_index != -1)
         {
            frame_used_[old_frame_index] = FR_FREE;
         }*/
         frame_index = frame_queue_[0];
         nb_frame_in_queue_--;
         memmove (frame_queue_, &frame_queue_[1], nb_frame_in_queue_*sizeof(unsigned int));
         
         //if (frame_index != -1)
         {
            //logger_->Write("DIS", LogNotice, "Loop : display %i - nb_frame_in_queue_ : %i", frame_index, nb_frame_in_queue_);
            mutex_.Release();
            frame_buffer_->SetVirtualOffset(143, 47 / 2 + frame_index * 1024);
            frame_buffer_->WaitForVerticalSync();
            // Set it as available
            frame_used_[frame_index] = FR_FREE;
            //old_frame_index = frame_index;
         }
         /*else
         {
            mutex_.Release();
            logger_->Write("DIS", LogNotice, "No buffer to display");
            CTimer::Get()->MsDelay(1);
         }*/
      }
      else
      {
         mutex_.Release();
         CTimer::Get()->MsDelay(1);
      }
      // sleep ?
      
   }
}

void DisplayPi::VSync(bool dbg )
{
   // Set current frame as ready
   //logger_->Write("DIS", LogNotice, "VSync : Frame ready is %i", buffer_used_);

#ifndef USE_QEMU_SUGARPI
   bool clear_framebuffer = false;
   if (full_resolution_cached_ != full_resolution_)
   {
      clear_framebuffer = true;
      full_resolution_cached_ = full_resolution_;
      
   }
   
   if (true/*sync_on_frame_*/) // To turn on : Use the display core !
   {
      frame_buffer_->SetVirtualOffset(143, 47 / 2 + buffer_used_ * 1024);
      if (sync_on_frame_)
      {
         frame_buffer_->WaitForVerticalSync();
      }
      if (clear_framebuffer)
      {
         //unsigned char* line = (unsigned char*)(frame_buffer_->GetBuffer() + buffer_used_*1024*frame_buffer_->GetPitch());
         unsigned char* line = reinterpret_cast<unsigned char*>(frame_buffer_->GetBuffer() + buffer_used_*1024*frame_buffer_->GetPitch());
         for (unsigned int count = 0; count < 1024; count++)
         {
            memset(line, 0x0, 1024*4);
            line += frame_buffer_->GetPitch();
         }
      }

   }
   else
   {
      mutex_.Acquire();
      // get a new one (or wait for one to be free)
      bool found = false;

      for (int i = 0; i < FRAME_BUFFER_SIZE && !found; i++)
      {
         if (frame_used_[i] == FR_FREE)
         {
            frame_queue_[nb_frame_in_queue_++] = buffer_used_;
            frame_used_[buffer_used_] = FR_READY;
            //logger_->Write("DIS", LogNotice, "VSync : Add %i - nb_frame_in_queue_ : %i", buffer_used_, nb_frame_in_queue_);

            frame_used_[i] = FR_USED;
            buffer_used_ = i;
            found = true;
            break;
         }
      }
      if (!found)
         logger_->Write("DIS", LogNotice, "All buffers are used");
      mutex_.Release();
   }
#else
   frame_buffer_->SetVirtualOffset(143, 47 / 2  * 1024);
   frame_buffer_->WaitForVerticalSync();

#endif
   //added_line_ ^= 1;
   //buffer_num_ ^= 1;
   added_line_ = 1;

//   static unsigned int count = 0;
//   static unsigned int max_tick = 0;
//   static unsigned int nb_long_frame = 0;

   // Frame is ready

   // wait for a new frame to be available




   // If last frame is more than 20ms, just don't do it
   
   /*frame_buffer_->WaitForVerticalSync();
   unsigned int new_tick = timer_->GetClockTicks();

   if (new_tick - last_tick_frame_ > max_tick)
      max_tick = new_tick - last_tick_frame_;

   if (new_tick - last_tick_frame_ > 20500)
   {
      nb_long_frame++;
   }
  
   if (++count == 500)
   {
      logger_->Write("DIS", LogNotice, "500frame : max_frame : %i; Nb frames > 20ms : %i", max_tick, nb_long_frame);
      max_tick = 0;
      count = 0;
      nb_long_frame = 0;
   }
   last_tick_frame_ = new_tick;
   */

#define PROPTAG_BLANK_SCREEN	0x00040002
   /*CBcmPropertyTags Tags;
   TBlankScreen blankScreen;
   blankScreen.blank = 0;
   if (Tags.GetTag(PROPTAG_BLANK_SCREEN, &blankScreen, sizeof blankScreen, 4))
   {
   }
   else
   {
      logger_->Write("Display", LogNotice, "PROPTAG_BLANK_SCREEN - KO...");
   }
   
   */
   //logger_->Write("Display", LogNotice, "Vsync : added_line_=%i", added_line_);
}

// Start of sync
void DisplayPi::StartSync()
{
   logger_->Write("Display", LogNotice, "StartSync - NOT IMPLEMENTED ");
}

// Wait VBL
void DisplayPi::WaitVbl()
{
   frame_buffer_->WaitForVerticalSync();
}



int* DisplayPi::GetVideoBuffer(int y)
{
   if (!full_resolution_cached_)
   {
      y = y * 2 + added_line_;
   }
   

   y &= 0x3FF;
   y += buffer_used_ * 1024;

///// 
   //logger_->Write("Display", LogNotice, "GetVideoBuffer : y = %i; buffer_used_ : %i, ==>%i", y, buffer_used_, frame_buffer_->GetBuffer() + y * frame_buffer_->GetPitch());
   return reinterpret_cast<int*>(frame_buffer_->GetBuffer() + y * frame_buffer_->GetPitch());

//   return (int*)(frame_buffer_->GetBuffer() + (y * 2 /*+ added_line_*/)* frame_buffer_->GetPitch() );
   
      
}

void DisplayPi::Reset()
{
   logger_->Write("Display", LogNotice, "Reset - NOT IMPLEMENTED ");
}

void DisplayPi::FullScreenToggle()
{
   logger_->Write("Display", LogNotice, "FullScreenToggle - NOT IMPLEMENTED ");
}

void DisplayPi::ForceFullScreen(bool fullscreen)
{
   logger_->Write("Display", LogNotice, "ForceFullScreen - NOT IMPLEMENTED ");
}

void DisplayPi::Screenshot()
{
   logger_->Write("Display", LogNotice, "Screenshot - NOT IMPLEMENTED ");

}

void DisplayPi::ScreenshotEveryFrame(int on)
{
   logger_->Write("Display", LogNotice, "ScreenshotEveryFrame - NOT IMPLEMENTED ");
}

bool DisplayPi::IsEveryFrameScreened()
{
   logger_->Write("Display", LogNotice, "IsEveryFrameScreened - NOT IMPLEMENTED ");
   return false;
}

bool DisplayPi::SetSyncWithVbl(int speed)
{
   logger_->Write("Display", LogNotice, "SetSyncWithVbl - NOT IMPLEMENTED ");
   return true;
}

bool DisplayPi::IsWaitHandled()
{
   logger_->Write("Display", LogNotice, "IsWaitHandled - NOT IMPLEMENTED ");
   return true;
}

bool DisplayPi::IsDisplayed()
{
   return true;
}

bool DisplayPi::GetBlackScreenInterval()
{
   logger_->Write("Display", LogNotice, "GetBlackScreenInterval - NOT IMPLEMENTED ");
   return false;

}

void DisplayPi::SetBlackScreenInterval(bool on)
{
   logger_->Write("Display", LogNotice, "SetBlackScreenInterval - NOT IMPLEMENTED ");
}


void DisplayPi::WindowChanged(int x_in, int y_in, int wnd_width, int wnd_height)
{
   logger_->Write("Display", LogNotice, "WindowChanged - NOT IMPLEMENTED ");
}

void DisplayPi::ResetLoadingMedia()
{
   logger_->Write("Display", LogNotice, "ResetLoadingMedia - NOT IMPLEMENTED ");
}

void DisplayPi::SetLoadingMedia()
{
   logger_->Write("Display", LogNotice, "SetLoadingMedia - NOT IMPLEMENTED ");
}

void DisplayPi::ResetDragnDropDisplay()
{
   logger_->Write("Display", LogNotice, "ResetDragnDropDisplay - NOT IMPLEMENTED ");

}

void DisplayPi::SetDragnDropDisplay(int type)
{
   logger_->Write("Display", LogNotice, "SetDragnDropDisplay - NOT IMPLEMENTED ");
}

void DisplayPi::SetCurrentPart(int x, int y)
{
   logger_->Write("Display", LogNotice, "SetCurrentPart - NOT IMPLEMENTED ");
}

int DisplayPi::GetDnDPart()
{
   logger_->Write("Display", LogNotice, "GetDnDPart - NOT IMPLEMENTED ");
   return 0;
}


void DisplayPi::DisplayText(const char* txt, int x, int y, bool selected)
{
   // Display text
   int i = 0;

   char buff[16];
   memset(buff, 0, sizeof buff);
   strncpy(buff, txt, 15);
   
   unsigned int x_offset_output = 0;

   while (txt[i] != '\0' )
   {

      // Display character
      unsigned char c = txt[i];

      if ( c == ' ')
      {
         x_offset_output += 10;
      }
      else
      {
         // Look for proper bitmap position (on first line only)
         for (int display_y = 0; display_y < font_->GetLetterHeight(c); display_y++)
         {
            int* line = GetVideoBuffer(display_y + y);
            font_->CopyLetter(c, display_y, &line[x + x_offset_output]);
         }
         x_offset_output += font_->GetLetterLength(c);
      }
      i++;
      
   }
}