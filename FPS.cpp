// FPS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;

#include <Windows.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <stdio.h>

int nScreenWidth = 120;
int nScreenheight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0;

int main()
{
    // creating screen buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenheight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    wstring map;

    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#....###.......#";
    map += L"#....#.#.......#";
    map += L"#....#.#.......#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();


    //game loop
    while (1)
    {

        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();


        // control machine broke
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.7f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (0.7f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {

            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }



        for (int x = 0; x < nScreenWidth; x++) {

            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth) {

                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                //is ray like gone and far away
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {

                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    //ray isnt gone and faraway
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;

                        vector<pair<float, float>> p; // distance,dot

                        for (int tx = 0; tx <2; tx++)
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        //big fking ass lambda function
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair <float, float>& right) {return left.first < right.first; });

                        
                        //change fbound to fix the shit
                        float fBound = 0.005;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        //if (acos(p.at(2).second) < fBound) bBoundary = true;


                    }
                }

            }

            //lmao where the floor at
            int nCeiling = (float)(nScreenheight / 2.0) - nScreenheight / ((float)fDistanceToWall);
            int nFloor = nScreenheight - nCeiling;

            short nShade = ' ';
            short nCShade = ' ';


            if (fDistanceToWall <= fDepth / 4.0f)       nShade = 0x2588; // close
            else if (fDistanceToWall < fDepth / 3.0f)   nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)   nShade = 0x2592;
            else if (fDistanceToWall < fDepth)          nShade = 0x2591;
            else                                        nShade = ' ';    // far

            if (bBoundary)                              nShade = ' '; //no

            for (int y = 0; y < nScreenheight; y++) {

                if (y < nCeiling)
                    screen[y * nScreenWidth + x] = ' ';

                else if (y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else {
                    //woah fancy distance shader goes here
                    float b = 1.0f - (((float)y - nScreenheight / 2.0f) / ((float)nScreenheight / 2.0f));
                    if (b < 0.25)           nCShade = '#';
                    else if (b < 0.5)       nCShade = 'x';
                    else if (b < 0.75)      nCShade = '.';
                    else if (b < 0.9)       nCShade = '-';
                    else                    nCShade = ' ';
                    screen[y * nScreenWidth + x] = nCShade;
                    

                    //screen[y * nScreenWidth + x] = ' ';

                    
                }
            }
        }

        // dispay shti
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        //fkin map cus pahan cant get mastery 3
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++) {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + (nMapWidth - nx - 1)];
            }

        screen[((int)fPlayerY + 1) * nScreenWidth + (int)(nMapWidth - fPlayerX)] = 'P';

        screen[nScreenWidth * nScreenheight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenheight, { 0,0 }, &dwBytesWritten);

    }

    return 0;
}


