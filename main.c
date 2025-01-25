#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <process.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#define PI atan(1)*4

char lastMessage[100];

//circle maker
void drawCircle(SDL_Renderer *renderer, int x, int y, int radius){
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius)) SDL_RenderDrawPoint(renderer, x + dx, y + dy);
        }
    }
}
void delay(int number_of_seconds){
    int milli_seconds = 1000 * number_of_seconds;// Converting time into milli_seconds
    clock_t start_time = clock();// Storing start time
    while (clock() < start_time + milli_seconds);// looping till required time is not achieved
}

unsigned __stdcall ClientSession(void *data){//multithreading to stop timeout bug
    SOCKET ConnectSocket = (SOCKET)data;// Process the client.
    char message[]=" ";//blank message to send server
    int buffer;
    while (true){
        delay(240);//delay of 4 minutes
        buffer = send( ConnectSocket, message, (int)strlen(message), 0 );//sending server a blank message to keep connection going
        if (buffer == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
    }
    return 0;
}

unsigned __stdcall messageListener(void *data){//multithreading to receive messages
    SOCKET ConnectSocket = (SOCKET)data;// Process the client.
    char message[DEFAULT_BUFLEN];//message holder
    char sender[50];
    int buffer;
    while (true){
        recv(ConnectSocket, sender, DEFAULT_BUFLEN, 0);//getting the user
        recv(ConnectSocket, message, DEFAULT_BUFLEN, 0);//getting the message
        strcat(sender," : ");
        strcat(sender,message);
        memset(lastMessage,0,sizeof(lastMessage));
        strcpy(lastMessage,sender);
        memset(message, 0, sizeof(message));//resetting the message
        memset(sender, 0, sizeof(sender));//resetting the message
    }
    return 0;
}

const int width=1024,height=576;//screen proportions

int main(int argc,char **argv){
    strcpy(lastMessage,"");
    //SDL setup stuff
    bool quit = false;
    SDL_Event event;
    SDL_Init( SDL_INIT_EVERYTHING );
    IMG_Init(IMG_INIT_PNG);
    IMG_Init(IMG_INIT_JPG);
    TTF_Init();

    //winsock stuff
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;// connecting socket

    char message[DEFAULT_BUFLEN];//message holder

    char receive[DEFAULT_BUFLEN];//received message storage
    int buffer;

    // Initialize Winsock
    buffer = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (buffer != NO_ERROR) {
        wprintf(L"WSAStartup function failed with error: %d\n", buffer);
        return 1;
    }

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        wprintf(L"socket function failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    struct sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr("10.0.0.104");//where it's connecting
    clientService.sin_port = htons(27015);//port

    // Connect to server.
    buffer = connect(ConnectSocket, (SOCKADDR *) & clientService, sizeof (clientService));
    if (buffer == SOCKET_ERROR) {
        wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
        buffer = closesocket(ConnectSocket);
        if (buffer == SOCKET_ERROR)
            wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //setting up timeout prevention
    unsigned threadID;
    (HANDLE)_beginthreadex(NULL, 0, &ClientSession, (void*)ConnectSocket, 0, &threadID);

    //end of client setup stuff

    SDL_Window * window = SDL_CreateWindow("messaging app",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);//creating a window
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);//creating the renderer
    SDL_Surface * screen=SDL_CreateRGBSurface(0,width,height,32,0,0,0,0);
    TTF_Font* Sans = TTF_OpenFont("The Wild Breath of Zelda.otf", 24);//setting up the font
    SDL_Surface * icon=IMG_Load("penisforeskinicon.JPG");//icon
    SDL_Color black = {0, 0, 0};
    SDL_SetRenderDrawColor(renderer,20,0,123,0);

    char input[DEFAULT_BUFLEN];
    strcpy(input,"");
    char prompt[100];
    strcpy(prompt,"do you have an account? Enter Y for yes, or N for no" );

    char screenType[20];
    strcpy(screenType,"login");

    //login variables
    bool logger=false;
    char login[2][20];
    char accounter[1];

    char text[100];//message

    //universal screen rects
    SDL_Rect topbar;
    topbar.y=0;
    topbar.x=0;
    topbar.h=80;
    topbar.w=width;

    SDL_Rect titleRect; //create a rect
    titleRect.w = 100; // controls the width of the rect
    titleRect.h = 60; // controls the height of the rect
    titleRect.x = width/2 -titleRect.w/2;  //controls the rect's x coordinate
    titleRect.y = topbar.y+15; // controls the rect's y coordinte

    SDL_Rect bottombar;
    bottombar.y=height-20;
    bottombar.x=0;
    bottombar.h=20;
    bottombar.w=width;

    SDL_Rect sq1;
    sq1.w=100;
    sq1.h=60;
    sq1.x=topbar.x+20;
    sq1.y=topbar.y+10;

    SDL_Rect sq2;
    sq2.w=100;
    sq2.h=60;
    sq2.x=sq1.x+sq1.w+20;
    sq2.y=topbar.y+10;

    SDL_Rect sq3;
    sq3.w=100;
    sq3.h=60;
    sq3.x=sq2.x+sq2.w+20;
    sq3.y=topbar.y+10;

    SDL_Rect sq4;
    sq4.w=100;
    sq4.h=60;
    sq4.x=topbar.w-sq4.w-20;
    sq4.y=topbar.y+10;

    SDL_Rect sq5;
    sq5.w=100;
    sq5.h=60;
    sq5.x=sq4.x-sq5.w-20;
    sq5.y=topbar.y+10;

    SDL_Rect sq6;
    sq6.w=100;
    sq6.h=60;
    sq6.x=sq5.x-sq6.w-20;
    sq6.y=topbar.y+10;

    SDL_Rect squares[]={sq1,sq2,sq3,sq4,sq5,sq6};


    //login screen rects
    SDL_Rect loginSquare;
    loginSquare.w=400;
    loginSquare.h=250;
    loginSquare.x=width/2-loginSquare.w/2;
    loginSquare.y=height/2-loginSquare.h/2;

    SDL_Rect loginInput;
    loginInput.w=70;
    loginInput.h=70;
    loginInput.x=loginSquare.x+loginSquare.w/2 -loginInput.w/2;
    loginInput.y=loginSquare.h-loginInput.h/2;

    SDL_Rect promptBox;
    promptBox.w=10*strlen(prompt);
    promptBox.h=40;
    promptBox.x=loginSquare.x;
    promptBox.y=loginSquare.y-50;

    //messaging rects
    SDL_Rect personBox;
    personBox.w=width;
    personBox.h=40;
    personBox.x=0;
    personBox.y=topbar.h;

    SDL_Rect sendBox;
    sendBox.w=400;
    sendBox.h=550;
    sendBox.x=40;
    sendBox.y=height-10;

    SDL_Rect label;
    label.w=300;
    label.h=60;
    label.x=width/2-label.w/2;
    label.y=personBox.y+personBox.h+20;

    SDL_Rect m1;
    m1.w=300;
    m1.h=60;
    m1.x=width/2-m1.w/2;
    m1.y=height/2-m1.h/2+label.h;

    //Enable text input
    SDL_StartTextInput();
    SDL_Event windowEvent;
    SDL_SetWindowIcon(window, icon);
    while (true){
        if (SDL_PollEvent(&windowEvent)){
            if (SDL_QUIT==windowEvent.type){
                break;
            }
        }
        if( windowEvent.type == SDL_KEYDOWN ){
            //Handle backspace
            if( windowEvent.key.keysym.sym == SDLK_BACKSPACE && strlen(input) > 0 ){
                //lop off character
                input[strlen(input)-1]='\0';
            }

            //handling enter button
            if( windowEvent.key.keysym.sym == SDLK_RETURN && strlen(input) > 0 ){
                //account input
                if (strcmp(screenType,"login")==0&&strlen(accounter)==0&&strlen(input)!=0&&strcmp(prompt,"do you have an account? Enter Y for yes, or N for no")==0){
                    if (strlen(input)!=1){
                        memset(input,0,sizeof(input));
                        strcpy(input,"");
                        strcpy(accounter,input);
                    } else {
                        memset(accounter,0,sizeof(accounter));
                        strcpy(accounter,input);
                        accounter[0]=toupper(accounter[0]);
                    }
                    if (accounter[0]=='N'||accounter[0]=='Y'){
                        bool newUsername=false;
                        memset(input,sizeof(input),0);
                        strcpy(input,"");

                        memset(prompt,sizeof(prompt),0);
                        if (accounter[0]=='Y') strcpy(prompt,"enter your username");
                        else strcpy(prompt,"create an account, enter your username");
                        promptBox.w=10*strlen(prompt);
                        send( ConnectSocket, accounter, 1, 0 );//sending if user wants to make new account
                    } else {
                        memset(input,0,sizeof(input));
                        strcpy(input,"");
                        strcpy(accounter,input);
                    }
                    //login info if you do have an account
                } else if (strcmp(screenType,"login")==0&&strlen(input)!=0&&strcmp(prompt,"enter your username")==0){
                    strcpy(login[0],input);
                    send( ConnectSocket, login[0], sizeof(login[0]), 0 );//sending login info
                    memset(input,sizeof(input),0);
                    strcpy(input,"");
                    memset(prompt,sizeof(prompt),0);
                    strcpy(prompt,"enter your password");
                    promptBox.w=10*strlen(prompt);
                    //login info  if you do have an account password
                } else if (strcmp(screenType,"login")==0&&strlen(input)!=0&&strcmp(prompt,"enter your password")==0&&accounter[0]=='Y'){
                    strcpy(login[1],input);
                    send( ConnectSocket, login[1], sizeof(login[1]), 0 );//sending login info
                    memset(input,sizeof(input),0);
                    strcpy(input,"");
                    recv(ConnectSocket, accounter, 1, 0);
                    if (accounter[0]=='Y'){
                        memset(prompt,sizeof(prompt),0);
                        strcpy(prompt,"You're logged in!");
                        //message listening
                        unsigned listener;
                        (HANDLE)_beginthreadex(NULL, 0, &messageListener, (void*)ConnectSocket, 0, &listener);
                        promptBox.w=10*strlen(prompt);
                    } else {
                        memset(prompt,sizeof(prompt),0);
                        strcpy(prompt,"Wrong login");
                        promptBox.w=10*strlen(prompt);
                    }
                    memset(login[0], 0, sizeof(login[0]));//resetting the login info
                    memset(login[1], 0, sizeof(login[1]));//resetting the login info
                    memset(accounter, 0, sizeof(accounter));//resetting the login info
                    //login if you're making an account username
                } else if (strcmp(screenType,"login")==0&&strlen(input)!=0&&(strcmp(prompt,"create an account, enter your username")==0||strcmp(prompt,"username taken, please pick a new one")==0)){
                    memset(login[0], 0, sizeof(login[0]));//resetting the login info
                    strcpy(login[0],input);
                    send( ConnectSocket, login[0], sizeof(login[0]), 0 );//sending login info
                    char checker[strlen("okie-dokie")+1];
                    recv(ConnectSocket, checker, strlen("okie-dokie"), 0);
                    printf(checker);
                        if (strcmp(checker,"okie-dokie")==0){
                            memset(input,sizeof(input),0);
                            strcpy(input,"");
                            memset(prompt,sizeof(prompt),0);
                            strcpy(prompt,"enter your password");
                            promptBox.w=10*strlen(prompt);
                        } else {
                            memset(input,sizeof(input),0);
                            strcpy(input,"");
                            memset(prompt,sizeof(prompt),0);
                            strcpy(prompt,"username taken, please pick a new one");
                            promptBox.w=10*strlen(prompt);
                        }
                        //account creation for no account
                } else if (strcmp(screenType,"login")==0&&strlen(input)!=0&&strcmp(prompt,"enter your password")==0&&accounter[0]=='N'){
                    strcpy(login[1],input);
                    send( ConnectSocket, login[1], sizeof(login[1]), 0 );//sending login info
                    memset(input,sizeof(input),0);
                    strcpy(input,"");
                    memset(login[0], 0, sizeof(login[0]));//resetting the login info
                    memset(login[1], 0, sizeof(login[1]));//resetting the login info
                    memset(accounter, 0, sizeof(accounter));//resetting the login info
                    memset(prompt,sizeof(prompt),0);
                    strcpy(prompt,"account created");
                    promptBox.w=10*strlen(prompt);
                }
                else if (strcmp(screenType,"messaging")==0&&strlen(input)!=0&&strcmp(prompt,"enter the message you want to send")==0){
                    memset(text,0,sizeof(text));
                    strcpy(text,input);
                    memset(input,0,sizeof(input));
                    strcpy(input,"");
                    // Sending a message
                    send( ConnectSocket, text, (int)strlen(text), 0 );
                    memset(prompt,0,sizeof(prompt));
                    strcpy(prompt,"who do you want to message?");
                }
                else if (strcmp(screenType,"messaging")==0&&strlen(input)!=0&&strcmp(prompt,"who do you want to message?")==0){
                    char receiver[40];
                    strcpy(receiver,input);
                    send( ConnectSocket, receiver, (int)strlen(receiver), 0 );//message
                    memset(input,sizeof(input),0);
                    strcpy(input,"");
                    memset(prompt,sizeof(prompt),0);
                    strcpy(prompt,"enter the message you want to send");
                }
            }
        }
                    //Special text input event
        else if( windowEvent.type == SDL_TEXTINPUT ){
            //Not copy or pasting
            if( !( SDL_GetModState() & KMOD_CTRL && ( windowEvent.text.text[ 0 ] == 'c' || windowEvent.text.text[ 0 ] == 'C' || windowEvent.text.text[ 0 ] == 'v' || windowEvent.text.text[ 0 ] == 'V' ) ) )
                {
                            //Append character
                    if (strcmp(prompt,"do you have an account? Enter Y for yes, or N for no")==0&&strlen(input)==0) strcat( input,windowEvent.text.text);
                    else if (strcmp(prompt,"do you have an account? Enter Y for yes, or N for no")!=0) strcat( input,windowEvent.text.text);
                }
            }
        SDL_FillRect(screen,NULL,230219217);
        SDL_FillRect(screen,&topbar,122241255);
        SDL_FillRect(screen,&bottombar,122241255);

        //coloring in the boxes for the login screen
        if (strcmp(screenType,"login")==0){
            SDL_FillRect(screen,&loginSquare,122241255);
            SDL_FillRect(screen,&loginInput,255255255);
            SDL_FillRect(screen,&promptBox,122241255);
        }

        if (strcmp(screenType,"messaging")==0){
            SDL_FillRect(screen,&sendBox,255255255);
            SDL_FillRect(screen,&personBox,255255255);
        }

        SDL_Texture * texture=SDL_CreateTextureFromSurface(renderer, screen);
        SDL_RenderCopy(renderer, texture, NULL, NULL);//placing the background
        SDL_Surface* title = TTF_RenderText_Solid(Sans, "HERMES", black);//title
        SDL_Texture* titleText = SDL_CreateTextureFromSurface(renderer, title);
        SDL_RenderCopy(renderer, titleText, NULL, &titleRect);

        //text for login screen
        if (strcmp(screenType,"login")==0){
            char* promptBuf=prompt;
            SDL_Surface* psurface = TTF_RenderText_Solid(Sans, promptBuf, black);//prompt
            SDL_Texture* ptext = SDL_CreateTextureFromSurface(renderer, psurface);
            SDL_RenderCopy(renderer, ptext, NULL, &promptBox);

            char* inputBuf=input;
            SDL_Surface* isurface = TTF_RenderText_Solid(Sans, inputBuf, black);//prompt
            SDL_Texture* itext = SDL_CreateTextureFromSurface(renderer, isurface);
            //fixing the text resize bug
            SDL_Rect loginInputT;
            loginInputT.x=loginInput.x;
            loginInputT.y=loginInput.y;
            loginInputT.w=loginInput.w;
            loginInputT.h=loginInput.h;

            TTF_SetFontSize(Sans, 70);
            if (strlen(inputBuf)!=0) {
                loginInputT.w=isurface->w;
                loginInputT.h=isurface->h;
                if (strcmp(prompt,"do you have an account? Enter Y for yes, or N for no")!=0) {
                    loginInput.w=isurface->w;
                    loginInput.h=isurface->h;
                } else {
                    loginInput.w=70;
                    loginInput.h=70;
                }
                loginInput.x=loginSquare.x+loginSquare.w/2 -loginInput.w/2;
                loginInput.y=loginSquare.h-loginInput.h/2;
            }
            SDL_RenderCopy(renderer, itext, NULL, &loginInputT);

            SDL_FreeSurface(psurface);
            SDL_DestroyTexture(ptext);

            SDL_FreeSurface(isurface);
            SDL_DestroyTexture(itext);

        }

        //text for messaging screen
        if (strcmp(screenType,"messaging")==0){
            char* inputBuf=input;
            TTF_SetFontSize(Sans, 40);
            SDL_Surface* isurface = TTF_RenderText_Solid(Sans, inputBuf, black);//prompt
            SDL_Texture* itext = SDL_CreateTextureFromSurface(renderer, isurface);
            if (strlen(inputBuf)!=0){
                sendBox.w=isurface->w;
                sendBox.h=isurface->h;
            } else {
                sendBox.w=400;
                sendBox.h=50;
            }
            SDL_RenderCopy(renderer, itext, NULL, &sendBox);

            char* promptBuf=prompt;
            SDL_Surface* psurface = TTF_RenderText_Solid(Sans, promptBuf, black);//prompt
            SDL_Texture* ptext = SDL_CreateTextureFromSurface(renderer, psurface);
            SDL_RenderCopy(renderer, ptext, NULL, &personBox);

            char* labelText="last message received";
            SDL_Surface* lsurface = TTF_RenderText_Solid(Sans, labelText, black);//prompt
            SDL_Texture* ltext = SDL_CreateTextureFromSurface(renderer, lsurface);
            SDL_RenderCopy(renderer, ltext, NULL, &label);

            char* lastText=lastMessage;
            TTF_SetFontSize(Sans, 40-strlen(lastMessage)/30);
            SDL_Surface* ltsurface = TTF_RenderText_Solid(Sans, lastText, black);//prompt
            SDL_Texture* lttext = SDL_CreateTextureFromSurface(renderer, ltsurface);
            if (strlen(lastMessage)!=0){
                m1.w=ltsurface->w;
                m1.h=ltsurface->h;
                m1.x=width/2-m1.w/2;
                m1.y=height/2-m1.h/2+label.h;
            }
            TTF_SetFontSize(Sans, 70);

            SDL_RenderCopy(renderer, lttext, NULL, &m1);

            SDL_FreeSurface(isurface);
            SDL_DestroyTexture(itext);

            SDL_FreeSurface(psurface);
            SDL_DestroyTexture(ptext);

            SDL_FreeSurface(lsurface);
            SDL_DestroyTexture(ltext);

            SDL_FreeSurface(ltsurface);
            SDL_DestroyTexture(lttext);
        }

        for (int i=0;i<6;i++) drawCircle(renderer,squares[i].x+squares[i].w/2,squares[0].y+squares[0].h/2,30);
        SDL_RenderPresent(renderer);

        // Don't forget to free your surface and texture
        SDL_FreeSurface(title);
        SDL_DestroyTexture(titleText);
        SDL_DestroyTexture(texture);

        if (strcmp(prompt,"Wrong login")==0||strcmp(prompt,"account created")==0){
            SDL_Delay(500);
            memset(prompt,sizeof(prompt),0);
            strcpy(prompt,"do you have an account? Enter Y for yes, or N for no");
            promptBox.w=10*strlen(prompt);
            memset(input,sizeof(input),0);
            strcpy(input,"");
        }
        if  (strcmp(prompt,"You're logged in!")==0) {
            SDL_Delay(500);
            memset(screenType,0,sizeof(screenType));
            strcpy(screenType,"messaging");
            memset(prompt,sizeof(prompt),0);
            strcpy(prompt,"enter the message you want to send");
            bottombar.y-=50;
            bottombar.h+=50;
            sendBox.y-=50;
            sendBox.h-=200;
        }
    }
    //cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    shutdown(ConnectSocket, SD_SEND);
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}