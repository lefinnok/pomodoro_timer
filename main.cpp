#include <wchar.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <memory>
#include <ncurses.h>
#include <map>
#include <fstream>
#include <locale.h>
#include <codecvt>
#include <algorithm>
/*class Font{
    public:
        Font(char character, std::vector<std::vector<char>> font): character_(character), font_(font){};
        std::vector<std::vector<char>> get_font(){
            return font_;
        }
    private:
        char character_;
        std::vector<std::vector<char>> font_;
};*/

#define progress_bar_width 42

std::map<char,std::vector<std::vector<wchar_t>>> font;

void display(int originalSeconds, int elapsedSeconds, std::string title = ""){
    //std::cout << "Time remaining: " << remainingSeconds << " second(s)." << std::endl;
    clear();
    int remainingSeconds = originalSeconds - elapsedSeconds;
    if(remainingSeconds<=3){
        flash();
        beep();
    }
    
    float elapsed_percentage = (float)elapsedSeconds / (float)originalSeconds;
    //turn remaining seconds into minutes and seconds
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;
    //format minutes and seconds
    std::string minutes_str = std::to_string(minutes);
    std::string seconds_str = std::to_string(seconds);
    if(minutes < 10){
        minutes_str = "0" + minutes_str;
    }
    if(seconds < 10){
        seconds_str = "0" + seconds_str;
    }
    //print formatted time remaining
    int mx=0, my=0;
    getmaxyx(stdscr, my, mx);
    int mid_x = (int)((float)mx/2);
    int mid_y = (int)((float)my/2);
    int initial_x = mid_x - 20;
    int initial_y = mid_y - 3;
    std::string full_time = minutes_str + ":" + seconds_str;
    //move(20,0);
    //printw(full_time.c_str());
    int on_char = 0;
    for(int character_initial_x = 0; character_initial_x < 45; character_initial_x += 9){
        for(int line = 0; line < 6; line++){
            wchar_t wstr[] = { ' ',' ',' ',' ',' ',' ', L'\0' };
            for(int character = 0; character < 6; character ++){
                //move(initial_y+line, initial_x+character_initial_x+character);
                //addch(font[full_time[on_char]][line][character]);
                wstr[character] = font[full_time[on_char]][line][character];
            }
            mvaddwstr(initial_y+line, initial_x+character_initial_x, wstr + '\0');
            //move(initial_x+character_initial_x, initial_y+line);
        }
        on_char++;
    }
    //print title in wstring
    std::wstring wtitle(title.begin(), title.end());
    mvaddwstr(initial_y-1, initial_x, wtitle.c_str());

    //draw box around timer
    /*mvaddwstr(initial_y-3, initial_x-3, L"█");
    mvaddwstr(initial_y-3, initial_x+46, L"█");
    mvaddwstr(initial_y+8, initial_x-3, L"█");
    mvaddwstr(initial_y+8, initial_x+46, L"█");
    for(int i = -2; i < 46; i++){
        mvaddwstr(initial_y-3, initial_x+i, L"█");
        mvaddwstr(initial_y+8, initial_x+i, L"█");
    }
    for(int i = -2; i < 8; i++){
        mvaddwstr(initial_y+i, initial_x-3, L"█");
        mvaddwstr(initial_y+i, initial_x+46, L"█");
    }*/

    //draw progress bar
    int progress_bar_x = initial_x;
    int progress_bar_y = initial_y + 7;
    for(int i = 0; i < progress_bar_width; i++){
        if(i < progress_bar_width * elapsed_percentage){
            mvaddwstr(progress_bar_y, progress_bar_x+i, L"█");
        }else{
            mvaddwstr(progress_bar_y, progress_bar_x+i, L"░");
        }
    }
    //refresh screen
    


    refresh();
}

class TimerEvent{
    public:
        TimerEvent(int duration_seconds, std::string title): duration_seconds_(duration_seconds), title_(title){};
        int get_duration_seconds(){
            return duration_seconds_;
        }
        std::string get_title(){
            return title_;
        }
    private:
        int duration_seconds_;
        std::string title_;
};

class Timer{
    public:
        void add_timer_event(int duration_seconds, std::string title){
            std::cout << "Adding timer event for " << duration_seconds << " seconds..." << std::endl;
            timer_events.push_back(std::make_unique<TimerEvent>(duration_seconds, title));
        }
        void start(){
            for(std::unique_ptr<TimerEvent>& timer_event: timer_events){
                int duration_seconds = timer_event->get_duration_seconds();
                
                //std::cout << "Starting timer for " << duration_seconds << " seconds..." << std::endl;
                for (int i = 0; i < duration_seconds; ++i) {
                    std::thread display_thread(display, duration_seconds, i, timer_event->get_title());
                    display_thread.detach();
                    //display(duration_seconds,i);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    //std::cout << "Time elapsed: " << i+1 << " second(s)." << std::endl;
                    //display_thread.join();
                }
                
                //std::cout << "Timer complete!" << std::endl;
            }
        }
    private:
        std::vector<std::unique_ptr<TimerEvent>> timer_events;
};



int main(int argc, char** argv) {
    
    Timer timer;

    //parse arguments
    //check for options
    if (argc == 2) {
        timer.add_timer_event(std::stoi(argv[1]), "Timer");
    }else if(argc>2 && (argc - 1) % 2 == 0){
        //parse arguments in pairs, push into vector as timer events
        for (int i = 1; i < argc; i+=2) {
            //change argv[I+1] to uppercase
            std::string title = argv[i+1];
            std::transform(title.begin(), title.end(), title.begin(), ::toupper);
            //check if argv[i] is a number
            char *p = argv[i];
            while(*p != '\0'){
                if(!isdigit(*p)){
                    std::cout << "Error: " << argv[i] << " is not a number." << std::endl;
                    return 1;
                }
                p++;
            }

            timer.add_timer_event(std::stoi(argv[i]), title);
        }
    }else{
        std::cout << "Usage: timer <duration_seconds> <title> <duration_seconds> <title> ..." << std::endl;
        return 1;
    }

    //ncurses init
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(0);
    //WINDOW * win = newwin(10, 10, 0, 0);

    //read font.txt into font map
    std::wifstream font_file("font.txt");
    font_file.imbue(std::locale(font_file.getloc(),
          new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
    if(font_file.is_open()){
        
        std::vector<std::wstring> filebuf; // create a vector to store the lines

        std::wstring line;
        while(getline(font_file, line)){
            filebuf.push_back(line);
            //std::wcout<<line<<std::endl;
        }
        //return 0;
        //std::wcout<<filebuf.size()<<std::endl;
        for(int i = 0; i < filebuf.size(); i+=8){
            char name = (char)filebuf[i][0];
            //std::cout<<name<<std::endl;
            //this is going to be stupid :3
            std::vector<std::vector<wchar_t>> fontbuf(6,std::vector<wchar_t>(6));
            for(int interline = 1; interline <= 6; interline++){
                //std::cout<<filebuf[i+interline]<<std::endl;
                for(int character = 0; character < 6; character++){
                    if(character<filebuf[i+interline].size()){
                        fontbuf[interline-1][character] = filebuf[i+interline][character];
                    }else{
                        fontbuf[interline-1][character] = L' ';
                    }
                }
            }
            font.insert({name,fontbuf});
            //std::cout<<name<<std::endl;
        }
    }else{;
        std::cout << "Error: font.txt not found." << std::endl;
        return 1;
    }
    //return 0;

    /*for (auto const& x : font)
    {
        std::cout << x.first << std::endl;
        for(int i = 0; i < 6; i++){
            for(int j = 0; j < 6; j++){
                std::wcout << x.second[i][j];
            }
            std::cout << std::endl;
        }
        
    }*/

    //return 0;
    
    timer.start();
    endwin();
    return 0;
}