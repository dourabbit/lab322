#pragma once
#include "Stream2Analysis.hpp"
#include "pthread.h"
namespace lab322DotNet {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// MainFrm 摘要
	/// </summary>
	public ref class MainFrm : public System::Windows::Forms::Form
	{
	public:
		Graphics^ graphics;
		System::Drawing::Font^ font;
		System::Drawing::PointF point;
		System::Drawing::Brush^ brush;
			
		System::Windows::Forms::TextBox^  output;
		static MainFrm^ singleton;
		static MainFrm^ GetForm(){return singleton;};
		MainFrm(void)
		{
			InitializeComponent();
			wrapper = gcnew Stream2AnalysisWrapper();
			font = gcnew System::Drawing::Font( "Arial",10 );
			brush = System::Drawing::Brushes::Blue;
			point =  Point(30,30);
			singleton = this;
		}
	

	protected:
		/// <summary>
		/// 清理所有正在使用的资源。
		/// </summary>
		~MainFrm()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// 必需的设计器变量。
		/// </summary>
		System::ComponentModel::Container ^components;
		System::Windows::Forms::SplitContainer^  splitContainer1;
		System::Windows::Forms::Button^  StartBtn;
		System::Windows::Forms::Button^  StopBtn;
		Stream2AnalysisWrapper^ wrapper;
		PictureBox^ pictureBox1;
	#pragma region Windows Form Designer generated code
			/// <summary>
			/// 设计器支持所需的方法 - 不要
			/// 使用代码编辑器修改此方法的内容。
			/// </summary>
			void InitializeComponent(void)
			{
				this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
				this->StartBtn = (gcnew System::Windows::Forms::Button());
				this->output = (gcnew System::Windows::Forms::TextBox());
				this->StopBtn = (gcnew System::Windows::Forms::Button());
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->splitContainer1))->BeginInit();
				this->splitContainer1->Panel2->SuspendLayout();
				this->splitContainer1->SuspendLayout();
				this->SuspendLayout();
				// 
				// splitContainer1
				// 
				this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
				this->splitContainer1->Location = System::Drawing::Point(0, 0);
				this->splitContainer1->Name = L"splitContainer1";
				this->splitContainer1->Orientation = System::Windows::Forms::Orientation::Horizontal;
				// 
				// splitContainer1.Panel2
				// 
				this->splitContainer1->Panel2->Controls->Add(this->StopBtn);
				this->splitContainer1->Panel2->Controls->Add(this->StartBtn);
				this->splitContainer1->Panel2->Controls->Add(this->output);
				this->splitContainer1->Size = System::Drawing::Size(585, 382);
				this->splitContainer1->SplitterDistance = 333;
				this->splitContainer1->TabIndex = 0;
				// 
				// StartBtn
				// 
				this->StartBtn->Location = System::Drawing::Point(425, -2);
				this->StartBtn->Name = L"StartBtn";
				this->StartBtn->Size = System::Drawing::Size(75, 23);
				this->StartBtn->TabIndex = 0;
				this->StartBtn->Text = L"Start";
				this->StartBtn->UseVisualStyleBackColor = true;
				this->StartBtn->Click += gcnew System::EventHandler(this, &MainFrm::button1_Click);
				// 
				// output
				// 
				this->output->Location = System::Drawing::Point(0, 0);
				this->output->Multiline = true;
				this->output->Name = L"output";
				this->output->Size = System::Drawing::Size(100, 21);
				this->output->TabIndex = 0;
				// 
				// StopBtn
				// 
				this->StopBtn->Location = System::Drawing::Point(506, -2);
				this->StopBtn->Name = L"StopBtn";
				this->StopBtn->Size = System::Drawing::Size(75, 23);
				this->StopBtn->TabIndex = 1;
				this->StopBtn->Text = L"Stop";
				this->StopBtn->UseVisualStyleBackColor = true;
				// 
				// Form1
				// 
				this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
				this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
				this->ClientSize = System::Drawing::Size(585, 382);
				this->Controls->Add(this->splitContainer1);
				this->Name = L"Form1";
				this->Text = L"Form1";
				this->Closed += gcnew System::EventHandler(this, &MainFrm::Form1_Closed);
				this->Load += gcnew System::EventHandler(this, &MainFrm::Form1_Load);
				this->splitContainer1->Panel2->ResumeLayout(false);
				this->splitContainer1->Panel2->PerformLayout();
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->splitContainer1))->EndInit();
				this->splitContainer1->ResumeLayout(false);
				this->ResumeLayout(false);

			}
	#pragma endregion


		void Form1_Load( Object^ /*sender*/, System::EventArgs^ /*e*/ )
		{
			pictureBox1 = gcnew PictureBox;
			
			// Dock the PictureBox to the form and set its background to white.
			pictureBox1->Dock = DockStyle::Fill;
			pictureBox1->BackColor = Color::White;

			// Connect the Paint event of the PictureBox to the event handler method.
			pictureBox1->Paint += gcnew System::Windows::Forms::PaintEventHandler( this, &MainFrm::pictureBox1_Paint );


			this->output->Dock = DockStyle::Fill;
			this->output->BackColor = Color::White;

			this->splitContainer1->Panel1->Controls->Add(pictureBox1);
			this->splitContainer1->Panel2->Controls->Add(this->output);
		}
		void Form1_Closed( Object^ /*sender*/, System::EventArgs^ /*e*/ )
		{
		}
		void pictureBox1_Paint( Object^ /*sender*/, System::Windows::Forms::PaintEventArgs^ e )
		{
			// Create a local version of the graphics object for the PictureBox.
			Graphics^ g = e->Graphics;
			graphics = g;


			// Draw a string on the PictureBox.
			g->DrawString( "This is a diagonal line drawn on the control",font,brush , point );

			// Draw a line in the PictureBox.
			g->DrawLine( System::Drawing::Pens::Red, pictureBox1->Left, pictureBox1->Top,
				pictureBox1->Right, pictureBox1->Bottom );
		}
		System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
		}
	};
}
