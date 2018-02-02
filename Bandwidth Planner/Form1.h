#pragma once


namespace BandwidthPlanner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	protected: 
	private: System::Windows::Forms::Timer^  timer1;
	private: System::Windows::Forms::Button^  Refresh_Button;
	private: System::Windows::Forms::Timer^  timer2;
	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->Refresh_Button = (gcnew System::Windows::Forms::Button());
			this->timer2 = (gcnew System::Windows::Forms::Timer(this->components));
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			// 
			// pictureBox1
			// 
			this->pictureBox1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->pictureBox1->Location = System::Drawing::Point(10, 10);
			this->pictureBox1->Margin = System::Windows::Forms::Padding(0);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(1649, 949);
			this->pictureBox1->TabIndex = 0;
			this->pictureBox1->TabStop = false;
			this->pictureBox1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::pictureBox1_Paint);
			this->pictureBox1->Resize += gcnew System::EventHandler(this, &Form1::pictureBox1_Resize);
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 15000;
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
			// 
			// Refresh_Button
			// 
			this->Refresh_Button->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->Refresh_Button->Location = System::Drawing::Point(16, 909);
			this->Refresh_Button->Margin = System::Windows::Forms::Padding(6);
			this->Refresh_Button->Name = L"Refresh_Button";
			this->Refresh_Button->Size = System::Drawing::Size(150, 44);
			this->Refresh_Button->TabIndex = 1;
			this->Refresh_Button->Text = L"Refresh";
			this->Refresh_Button->UseVisualStyleBackColor = true;
			this->Refresh_Button->Click += gcnew System::EventHandler(this, &Form1::Refresh_Button_Click);
			// 
			// timer2
			// 
			this->timer2->Enabled = true;
			this->timer2->Interval = 5000;
			this->timer2->Tick += gcnew System::EventHandler(this, &Form1::timer2_Tick);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(12, 25);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1669, 969);
			this->Controls->Add(this->Refresh_Button);
			this->Controls->Add(this->pictureBox1);
			this->Margin = System::Windows::Forms::Padding(6);
			this->Name = L"Form1";
			this->Padding = System::Windows::Forms::Padding(10);
			this->Text = L"Bandwidth Planner";
			this->FormClosed += gcnew System::Windows::Forms::FormClosedEventHandler(this, &Form1::Form1_FormClosed);
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion

	private: System::String^ TrimFloat(float v, int places)
	{
		if (places <= 0) return Math::Round(v).ToString();
		return Math::Round(v, places).ToString();

		float mult = 10.0f;
		mult = (float)Math::Pow(10.0, (double)places);

		return ((float)(int)(v * mult) / mult).ToString();
	}

			 double avg_speed = 0;
			 unsigned long long lastcalcavgspeed = 0;
			 float lastusedavgspeed = 0;

	static DateTime time_t2DateTime(unsigned long long date) {
		double sec = static_cast<double>(date);
		DateTime dt = DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind::Utc).AddSeconds(sec);
		return dt.ToLocalTime();
	}

	private: System::Void CreateCalendar(Drawing::Graphics ^gfx)
			 {
				 float boxwidth = 120.0f;
				 float boxheight = 60.0f;
				 boxwidth = this->pictureBox1->Width / 7.01f;
				 boxheight = this->pictureBox1->Height / 8.0f;

				 Drawing::Font ^tfont = gcnew Drawing::Font(SystemFonts::DefaultFont->Name, 12.0f);
				 Drawing::Brush ^tbrush = gcnew SolidBrush(Color::Black);
				 //Drawing::StringFormat ^center = gcnew Drawing::StringFormat;
				 //center->Alignment = Drawing::StringAlignment::Center;

				 BillingCycle cycle( BillingDay(billstartyear, billstartmonth, billstartday),
					 BillingDay(billendyear, billendmonth, billendday) );

				 int days = cycle.NumDays();
				 int today = System::DateTime::Now.Day;
				 if(System::DateTime::Now.Month != cycle.startday.month)
					 today+= cycle.startday.daysinmonth;

				 for(int i=0;i<7;i++)
				 {
					 gfx->DrawString(((System::DayOfWeek)i).ToString(), tfont, tbrush, boxwidth*(float)i, 40.0f);
					 gfx->DrawString(TrimFloat(DailyUsage[i], 1) + L" GB", tfont, tbrush, boxwidth*(float)i, 60.0f);
				 }

				 float BudgetedGBs = 0.0f;
				 float TDailyUsage[7] = {DailyUsage[0],DailyUsage[1],DailyUsage[2],DailyUsage[3],DailyUsage[4],DailyUsage[5],DailyUsage[6]};

				 //float daycount[7] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
				 float totaldailyusage=0.0f;
				 //int numday[7] = {0,0,0,0,0,0,0};

				 for(auto d=cycle.begin();d!=cycle.end();d++)
				 {
					 totaldailyusage += DailyUsage[d.dow];
				 }
				 for(int i=0;i<7;i++)
				 {
					 TDailyUsage[i] *= cap / totaldailyusage;
				 }
				 float totalallowance = 0.0f;

				 /*for(int i=0;i<7;i++)
				 {
					 gfx->DrawString(((System::DayOfWeek)i).ToString(), tfont, tbrush, boxwidth*(float)i, 40.0f);
					 gfx->DrawString(((float)(int)(DailyUsage[i]*10.0f)/10.0f).ToString() + L" GB", tfont, tbrush, boxwidth*(float)i, 60.0f);
				 }*/

				 for(auto d=cycle.begin();d!=cycle.end();d++)
				 {
					 totalallowance += TDailyUsage[d.dow];

					 float ftoday = used - BudgetedGBs;
					 ftoday = Math::Max(0.0f, ftoday);
					 ftoday = Math::Min(TDailyUsage[d.dow], ftoday);
					 BudgetedGBs += TDailyUsage[d.dow];

					 if (d.day == System::DateTime::Now.Day && d.month == System::DateTime::Now.Month) {
						 gfx->FillRectangle(Brushes::Yellow, d.dow*boxwidth, boxheight*(d.week + 1), boxwidth, boxheight / 2.0f);
						 
						 if (usedtoday > TDailyUsage[d.dow]) {
							 taskbarglob.SetProgressState((HWND)(int)this->Handle, TBPF_ERROR);
							 taskbarglob.SetProgressValue((HWND)(int)this->Handle, (ULONGLONG)(usedtoday*100.0f), (ULONGLONG)(TDailyUsage[d.dow] * 200.0f));
						 }
						 else if (usedtoday > TDailyUsage[d.dow] * 0.75f) {
							 taskbarglob.SetProgressState((HWND)(int)this->Handle, TBPF_PAUSED);
							 taskbarglob.SetProgressValue((HWND)(int)this->Handle, (ULONGLONG)(usedtoday*100.0f), (ULONGLONG)(TDailyUsage[d.dow] * 100.0f));
						 }
						 else {
							 taskbarglob.SetProgressState((HWND)(int)this->Handle, TBPF_NORMAL);
							 taskbarglob.SetProgressValue((HWND)(int)this->Handle, (ULONGLONG)(usedtoday*100.0f), (ULONGLONG)(TDailyUsage[d.dow] * 100.0f));
						 }
					 }

					 if(BudgetedGBs < used)
						 gfx->FillRectangle(Brushes::LightGreen, d.dow*boxwidth, boxheight*(d.week+1)+boxheight/2.0f, boxwidth, boxheight/2.0f);
					 else if( BudgetedGBs-TDailyUsage[d.dow] < used && ftoday>0.0f)
					 {
						 float ratio = ftoday / TDailyUsage[d.dow];
						 gfx->FillRectangle(Brushes::LightGreen, d.dow*boxwidth, boxheight*(d.week+1)+boxheight/2.0f, (boxwidth*ratio), boxheight/2.0f);
					 }
					 else if(ftoday==0.0f)
					 {
					 }

					 gfx->DrawString( 
						 TrimFloat(ftoday, 1)
						 + L"/" + 
						 TrimFloat(TDailyUsage[d.dow], 1)
						 + L" GB", tfont, tbrush, (float)d.dow*boxwidth, boxheight*(float)(d.week+1)+boxheight/2.0f);

					 gfx->DrawRectangle(Drawing::Pens::Black, d.dow*boxwidth, boxheight*(d.week+1), boxwidth, boxheight);
					 //gfx->DrawString( d.day.ToString(), tfont, tbrush, (float)d.dow*boxwidth, boxheight*(float)(d.week+1));
					 gfx->DrawString(d.year.ToString()+"-"+d.month.ToString()+"-"+d.day.ToString(), tfont, tbrush, (float)d.dow*boxwidth, boxheight*(float)(d.week + 1));
				 }

				 if (used == 0)
				 {

				 }
				 else if ( (lastcalcavgspeed == 0 && used>0) || lastupdate - lastcalcavgspeed > 600) {
					 lastcalcavgspeed = lastupdate;
					 lastusedavgspeed = used;
				 }
				 else if (lastusedavgspeed < used && lastupdate - lastcalcavgspeed > 5) {
					 double div = 300.0 / (double)(lastupdate - lastcalcavgspeed);
					 avg_speed -= avg_speed / div;
					 double curr_speed = (double)(used - lastusedavgspeed) / (double)(lastupdate - lastcalcavgspeed);
					 if (avg_speed == 0.0) avg_speed = curr_speed;
					 avg_speed += curr_speed / div;
					 lastcalcavgspeed = lastupdate;
					 lastusedavgspeed = used;
				 }

				 gfx->DrawString(time_t2DateTime(lastupdate).ToShortTimeString() + L"      " + TrimFloat(used, 2) + L" GB of " + TrimFloat(cap, 2) + L" GB      today " + TrimFloat(usedtoday, 3) + L" GB of " + TrimFloat(TDailyUsage[(int)DateTime::Now.DayOfWeek], 3) + L" GB      speed: " +TrimFloat(avg_speed*(1024.0), 3) + L" MB/s", tfont, tbrush, 10.0f, 10.0f);
			 }

	private: System::Void CreateCalendarOld(Drawing::Graphics ^gfx)
			 {
				 float boxwidth = 100.0f;
				 float boxheight = 50.0f;
				 //gfx->CompositingQuality = Drawing2D::CompositingQuality::HighQuality;
				 //gfx->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;
				 Drawing::Font ^tfont = gcnew Drawing::Font(SystemFonts::DefaultFont->Name, 12.0f);
				 Drawing::Brush ^tbrush = gcnew SolidBrush(Color::Black);
				 //gfx->TextRenderingHint = Drawing::Text::TextRenderingHint::SystemDefault;
				 //gfx->DrawRectangle(System::Drawing::Pens::Black, 10, 10, 100, 100);
				 //gfx->DrawRectangle( gcnew Pen(Color::Black, 10.0f), 10, 10, 100, 100);

				 int daysinmonth = System::DateTime::DaysInMonth( billstartyear, billstartmonth);
				 int days = daysinmonth-billstartday;
				 if(billstartmonth!=billendmonth)
					 days += billendday;
				 else
					 days = billendday-billstartday;

				 /*System::DayOfWeek*/
				 //int dow = (int)System::DateTime::DateTime(System::DateTime::Now.Year, System::DateTime::Now.Month, 1).DayOfWeek;
				 int dow2 = (int)System::DateTime::DateTime(billstartyear, billstartmonth, billstartday).DayOfWeek;
				 int today = System::DateTime::Now.Day;
				 if(System::DateTime::Now.Month != billstartmonth)
					 today+= daysinmonth;

				 for(int i=0;i<7;i++)
				 {
					 gfx->DrawString(((System::DayOfWeek)i).ToString(), tfont, tbrush, boxwidth*(float)i, 40.0f);
					 gfx->DrawString(((float)(int)(DailyUsage[i]*10.0f)/10.0f).ToString() + L" GB", tfont, tbrush, boxwidth*(float)i, 60.0f);
					 //gfx->DrawString(day[i],
					 //gfx->DrawLine(
				 }

				 float BudgetedGBs = 0.0f;
				 float TDailyUsage[7] = {DailyUsage[0],DailyUsage[1],DailyUsage[2],DailyUsage[3],DailyUsage[4],DailyUsage[5],DailyUsage[6]};

				 float daycount[7] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
				 float totaldailyusage=0.0f;
				 int numday[7] = {0,0,0,0,0,0,0};

				 for(int i=billstartday,day=dow2,week=1;i<=days;i++, day=(day+1)%7)
				 {
					 daycount[day]+=TDailyUsage[day];
					 numday[day]++;
					 //totaldailyusage+=TDailyUsage[day];
					 if(day==6)
						 week++;
				 }
				 for(int i=0;i<7;i++)
				 {
					 totaldailyusage+=TDailyUsage[i];
				 }
				 for(int i=0;i<7;i++)
				 {
					 //TDailyUsage[i] = ((TDailyUsage[i] / totaldailyusage) * cap) / (float)numday[i];
					 TDailyUsage[i] = (TDailyUsage[i] / totaldailyusage) / ((float)days / 7.0f);
					 TDailyUsage[i] *= cap;
				 }

				 for(int i=billstartday,day=dow2,week=1;i<days+billstartday;i++, day=(day+1)%7)
				 {
					 if(i>=billstartday)
					 {
						 if (i == today)
						 {
							 gfx->FillRectangle(Brushes::Yellow, day*boxwidth, boxheight*(week + 1), boxwidth, boxheight / 2.0f);
							 taskbarglob.SetProgressState((HWND)(int)this->Handle, TBPF_NORMAL);
							 taskbarglob.SetProgressValue((HWND)(int)this->Handle, 50, 100);
						 }

						 float ftoday = used - BudgetedGBs;
						 ftoday = Math::Max(0.0f, ftoday);
						 ftoday = Math::Min(TDailyUsage[day], ftoday);
						 BudgetedGBs += TDailyUsage[day];

						 if(BudgetedGBs < used)
							 gfx->FillRectangle(Brushes::LightGreen, day*boxwidth, boxheight*(week+1)+boxheight/2.0f, boxwidth, boxheight/2.0f);
						 else if( BudgetedGBs-TDailyUsage[day] < used && ftoday>0.0f)
						 {
							 float ratio = ftoday / TDailyUsage[day];
							 gfx->FillRectangle(Brushes::LightGreen, day*boxwidth, boxheight*(week+1)+boxheight/2.0f, (boxwidth*ratio), boxheight/2.0f);
						 }
						 else if(ftoday==0.0f)
						 {
							 //gfx->FillRectangle(Brushes::Black, day*100, 40*(week+1)+20, (int)(100.0f), 20);
						 }

						 gfx->DrawString( 
							 (((float)(int)(ftoday*10.0f))/10.0f).ToString()
							 + L"/" + 
							 ((float)(int)(TDailyUsage[day]*10.0f)/10.0f).ToString()
							 + L" GB", tfont, tbrush, (float)day*boxwidth, boxheight*(float)(week+1)+boxheight/2.0f);
					 }

					 gfx->DrawRectangle(Drawing::Pens::Black, day*boxwidth, boxheight*(week+1), boxwidth, boxheight);
					 gfx->DrawString( ((i-1)%daysinmonth +1).ToString(), tfont, tbrush, (float)day*boxwidth, boxheight*(float)(week+1));
					 //gfx->DrawString( ((float)(int)(DailyUsage[day]*10.0f)/10.0f).ToString() + L"GB", tfont, tbrush, (float)day*boxwidth + 40.0f, boxheight*(float)(week+1));

					 if(day==6)
						 week++;
				 }
				 gfx->DrawString(System::DateTime::Now.ToString() + L"\t\t" + used.ToString()->Substring(0, 5) + L" GB used of " + cap.ToString()->Substring(0, 5) + L" GB - today " + usedtoday.ToString()->Substring(0, 5) + L"GB", tfont, tbrush, 10.0f, 10.0f);
			 }

	private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {
				 //CreateCalendar();
				 int dayofweek = (int)System::DateTime::DateTime(System::DateTime::Now.Year, System::DateTime::Now.Month,System::DateTime::Now.Day).DayOfWeek;
				 UpdateUsage(dayofweek);
				 auto timage = gcnew Drawing::Bitmap(100,100);
				 CreateCalendar( Drawing::Graphics::FromImage(timage) );
				 this->pictureBox1->Refresh();
			 }
	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
				 //CreateCalendar();
				 this->Show();
				 int dayofweek = (int)System::DateTime::DateTime(System::DateTime::Now.Year, System::DateTime::Now.Month, System::DateTime::Now.Day).DayOfWeek;
				 UpdateUsage(dayofweek);
			 }
	private: System::Void pictureBox1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
				 CreateCalendar(e->Graphics);
			 }
	private: System::Void Refresh_Button_Click(System::Object^  sender, System::EventArgs^  e) {
				 timer1_Tick(this, System::EventArgs::Empty);
			 }
private: System::Void Form1_FormClosed(System::Object^  sender, System::Windows::Forms::FormClosedEventArgs^  e) {
			 SaveUsageStats();
			 //Windows::Forms::MessageBox::Show(L"Usage stats saved!");
		 }
	private: System::Void pictureBox1_Resize(System::Object^  sender, System::EventArgs^  e) {
			this->pictureBox1->Refresh();
		}
	private: System::Void timer2_Tick(System::Object^  sender, System::EventArgs^  e) {
			if(used > 0) this->timer2->Enabled = false;
			this->pictureBox1->Refresh();
		}
};


/*[DllImport("user32.dll")]
extern int SendMessage(int hWnd, IntPtr msg, int wParam, int lParam);

static public ref class TaskbarProgress
{
public: enum TaskbarStates
{
	NoProgress = 0,
	Indeterminate = 0x1,
	Normal = 0x2,
	Error = 0x4,
	Paused = 0x8
};

	static System::Void SetState(IntPtr windowHandle, TaskbarStates taskbarState)
	{
		SendMessage((int)windowHandle, 1, 1, 1);
		//Message ^msg = Message::Create(windowHandle, 1, windowHandle, windowHandle);
		//if (taskbarSupported) taskbarInstance.SetProgressState(windowHandle, taskbarState);
	}

	static System::Void SetValue(IntPtr windowHandle, double progressValue, double progressMax)
	{
		//if (taskbarSupported) taskbarInstance.SetProgressValue(windowHandle, (ulong)progressValue, (ulong)progressMax);
	}
};*/
}
