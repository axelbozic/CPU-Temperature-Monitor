#include <iostream>
#include <chrono>
#include <thread>
#include <Windows.h>

#define SB_VERT

// Called to set displayed console text location
void goto_XY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Called to conceal maximize button from menu
void GreyMaxBox() {
    HWND hwnd = GetConsoleWindow();
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_MAXIMIZEBOX;
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

// Called to conceal minimize button from menu
void GreyMinBox() {
    HWND hwnd = GetConsoleWindow();
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_MINIMIZEBOX;
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

// Called to maintain specific screen size
void ScreenSize() {
    const int maxwidth = 180, maxheight = 37;
    if (System::Console::WindowWidth > System::Console::LargestWindowWidth - maxwidth) {
        System::Console::WindowWidth = System::Console::LargestWindowWidth - maxwidth;
    }
    if (System::Console::WindowHeight > System::Console::LargestWindowHeight - maxheight) {
        System::Console::WindowHeight = System::Console::LargestWindowHeight - maxheight;
    }
}

// Called to conceal both horizontal and vertical scrollbars
void ScrollBar() {
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsole, &screenBufferInfo);
    COORD new_screen_buffer_size;
    new_screen_buffer_size.X = screenBufferInfo.srWindow.Right -
        screenBufferInfo.srWindow.Left + 1;
    new_screen_buffer_size.Y = screenBufferInfo.srWindow.Bottom -
        screenBufferInfo.srWindow.Top + 1;
    SetConsoleScreenBufferSize(hConsole, new_screen_buffer_size);
}

// Called to receive information from sensors
public ref class UpdateVisitor_ : OpenHardwareMonitor::Hardware::IVisitor {
public:
    virtual void VisitComputer(OpenHardwareMonitor::Hardware::IComputer^ computer) {
        computer->Traverse(this);
    }
public:
    virtual void VisitHardware(OpenHardwareMonitor::Hardware::IHardware^ hardware) {
        hardware->Update();
        for each (OpenHardwareMonitor::Hardware::IHardware ^ subHardware in hardware->SubHardware) {
            subHardware->Accept(this);
        }
    }
public:
    virtual void VisitSensor(OpenHardwareMonitor::Hardware::ISensor^ sensor) { }
public:
    virtual void VisitParameter(OpenHardwareMonitor::Hardware::IParameter^ parameter) { }
};

// Receives data from UpdateVisitor_ and displays required information
void FetchCpuTemp() {
    float cpu_AVG_COUNTER = 0;
    System::Decimal cpu_AVG_DISP;
    UpdateVisitor_^ updateVisitor = gcnew UpdateVisitor_();
    OpenHardwareMonitor::Hardware::Computer^ computer = gcnew OpenHardwareMonitor::Hardware::Computer();
    computer->Open();
    computer->CPUEnabled = true;
    computer->Accept(updateVisitor);
    for (int i = 0; i < computer->Hardware->Length; i++) {
        if (computer->Hardware[i]->HardwareType == OpenHardwareMonitor::Hardware::HardwareType::CPU) {
            for (int j = 0; j < computer->Hardware[i]->Sensors->Length; j++) {
                if (computer->Hardware[i]->Sensors[j]->SensorType == OpenHardwareMonitor::Hardware::SensorType::Temperature) {
                    goto_XY(1, j-8);
                    System::Console::BackgroundColor = System::ConsoleColor::Black;
                    System::Console::ForegroundColor = System::ConsoleColor::DarkYellow;
                    System::Console::Write(computer->Hardware[i]->Sensors[j]->Name + ":  ");
                    System::Console::BackgroundColor = System::ConsoleColor::Black;
                    System::Console::ForegroundColor = System::ConsoleColor::Cyan;
                    System::Console::Write(computer->Hardware[i]->Sensors[j]->Value + " °C\n");
                    cpu_AVG_COUNTER += (float)computer->Hardware[i]->Sensors[j]->Value;
                    if (computer->Hardware[i]->Sensors[j]->Name == "CPU Package") {
                        cpu_AVG_COUNTER /= 9;
                        cpu_AVG_DISP = System::Math::Round((System::Decimal)cpu_AVG_COUNTER, 1);
                        System::Console::BackgroundColor = System::ConsoleColor::Black;
                        System::Console::ForegroundColor = System::ConsoleColor::DarkYellow;
                        System::Console::Write(" Average CPU Temp: ");
                        System::Console::BackgroundColor = System::ConsoleColor::Black;
                        System::Console::ForegroundColor = System::ConsoleColor::Cyan;
                        System::Console::Write(cpu_AVG_DISP + " °C");
                    }
                    System::Console::Out->Flush();
                }
            }
        }
    }
    cpu_AVG_COUNTER = 0;
    computer->Close();
}

int main() {
    while (true) {
        SetConsoleTitleW(L"CPU Temperature Monitor");
        ScreenSize();
        GreyMinBox();
        GreyMaxBox();
        ScrollBar();
        FetchCpuTemp();
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        System::Console::Clear();
    }
}