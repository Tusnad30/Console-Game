#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>


#define WIN_WIDTH 119 // 119
#define WIN_HEIGHT 29 // 29

#define FRAME_DELTA 0.016f // 0.016f

#define RAY_STEP_SIZE 0.03f // 0.03f
#define MAX_RAY_STEPS 1000 // 1000

#define CAM_FOV 1.0f // 1.0f
#define CAM_SPEED 1.0f // 1.0f
#define CAM_SENS 4.0f // 4.0f

#define LIGHT_RANGE 2.0f // 2.0f

#define MAX_FLOOR_DIST 2.5f // 2.5f
#define MAX_EXIT_DIST 5.0f // 5.0f

#define MAP_SIZE 25 // 25

// Debug
#define SHOW_MAP 0


HANDLE hConsole;
HANDLE hHeap;

unsigned int screen_buffer_size = WIN_WIDTH * WIN_HEIGHT + WIN_HEIGHT;
char* screen_buffer;

void clearScreenBuffer(char character);
void setPixel(unsigned int pos_x, unsigned int pos_y, char character);


char* map_data;

const char char_ramp[] = ".:;iX@";

float cam_pos_x = 2.5f;
float cam_pos_y = 2.5f;
float cam_rot = 0.0f;


void generateMap();
void render();
void processInput();

#if SHOW_MAP
void showMap();
#endif

int main()
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    hHeap = GetProcessHeap();

    screen_buffer = HeapAlloc(hHeap, 0, screen_buffer_size);
    map_data = HeapAlloc(hHeap, 0, MAP_SIZE * MAP_SIZE);

    generateMap();

    #if SHOW_MAP
    showMap();
    getchar();
    #endif

    while (1)
    {
        if (map_data[(int)cam_pos_y * MAP_SIZE + (int)cam_pos_x] == 2)
        {
            MessageBoxA(
                NULL,
                "You escaped the maze!",
                "Congradulations!",
                MB_ICONINFORMATION
            );
            break;
        }


        processInput();
        clearScreenBuffer(' ');

        render();

        DWORD num_written;
        WriteConsoleA(hConsole, screen_buffer, screen_buffer_size, &num_written, NULL);
        Sleep((DWORD)(FRAME_DELTA * 1000));
    }

    HeapFree(hHeap, 0, map_data);
    HeapFree(hHeap, 0, screen_buffer);
    
    return 0;
}


float randomUniform()
{
    return (float)rand() / RAND_MAX;
}


void generateMap()
{
    srand(GetTickCount());

    int cur_x = (int)(randomUniform() * (MAP_SIZE - 2)) + 1;
    int cur_y = 1;

    cam_pos_x = cur_x + 0.5f;
    cam_pos_y = cur_y + 0.5f;

    memset(map_data, 1, MAP_SIZE * MAP_SIZE);

    map_data[cur_y * MAP_SIZE + cur_x] = 0;

    for (int y = 0; y < MAP_SIZE - 2; y++)
    {
        for (int x = 0; x < MAP_SIZE - 2; x++)
        {
            if (randomUniform() < 0.66f)
                map_data[(y + 1) * MAP_SIZE + x + 1] = 0;
        }
    }

    unsigned int try_count = 0;

    while (1)
    {
        int test_x = cur_x;
        int test_y = cur_y;

        float test_rand = randomUniform();

        if (test_rand < 0.25f)
            test_x--;
        else if (test_rand < 0.5f)
            test_x++;
        else if (test_rand < 0.75f)
            test_y--;
        else
            test_y++;
        
        if (test_x > 0 && test_x < MAP_SIZE - 1 && test_y > 0 && test_y < MAP_SIZE - 1)
        {
            if (map_data[test_y * MAP_SIZE + test_x] == 0 || try_count >= 10)
            {
                try_count = 0;
                cur_x = test_x;
                cur_y = test_y;
                map_data[cur_y * MAP_SIZE + cur_x] = 0;
                
                if (cur_y == MAP_SIZE - 2)
                {
                    map_data[cur_y * MAP_SIZE + cur_x] = 2;
                    return;
                }
            }
            else
                try_count++;
        }
    }
}

void render()
{
    for (int i = 0; i < WIN_WIDTH; i++)
    {
        float ray_pos_x = cam_pos_x;
        float ray_pos_y = cam_pos_y;
        float ray_dist = 0.0f;

        const float ray_rot = cam_rot + ((float)i / WIN_WIDTH - 0.5f) * CAM_FOV;
        const float ray_dir_x = sinf(ray_rot) * RAY_STEP_SIZE;
        const float ray_dir_y = cosf(ray_rot) * RAY_STEP_SIZE;
        const float ray_step = RAY_STEP_SIZE * cosf(ray_rot - cam_rot);

        int line_height;
        int line_offset;
        char brightness_char;

        for (int j = 0; j < MAX_RAY_STEPS; j++)
        {
            int ray_prev_x = (int)ray_pos_x;
            int ray_prev_y = (int)ray_pos_y;
            ray_pos_x += ray_dir_x;
            ray_pos_y += ray_dir_y;
            ray_dist += ray_step;

            brightness_char = char_ramp[(int)((1.0f / (1.0f + ray_dist * ray_dist * (1.0f / LIGHT_RANGE))) * (sizeof(char_ramp) - 1))];

            line_height = (int)min((1.0f / ray_dist) * WIN_HEIGHT, WIN_HEIGHT + 1);
            line_offset = (WIN_HEIGHT - line_height) / 2;

            //if (ray_pos_x < 0.0f || ray_pos_x >= MAP_SIZE || ray_pos_y < 0.0f || ray_pos_y >= MAP_SIZE)
            //    break;

            if (map_data[(int)ray_pos_y * MAP_SIZE + (int)ray_pos_x] == 1)
                break;

            if ((ray_prev_x != (int)ray_pos_x || ray_prev_y != (int)ray_pos_y) && ray_dist < MAX_FLOOR_DIST)
                setPixel(i, (line_height - 1) + line_offset, brightness_char);

            if (map_data[(int)ray_pos_y * MAP_SIZE + (int)ray_pos_x] == 2 && ray_dist < MAX_EXIT_DIST)
                setPixel(i, (line_height - 1) + line_offset, '@');
        }

        for (int j = 0; j < line_height; j++)
            setPixel(i, j + line_offset, brightness_char);
    }
}


void processInput()
{
    float last_pos_x = cam_pos_x;
    float last_pos_y = cam_pos_y;

    if (GetAsyncKeyState('W'))
    {
        cam_pos_x += sinf(cam_rot) * CAM_SPEED * FRAME_DELTA;
        cam_pos_y += cosf(cam_rot) * CAM_SPEED * FRAME_DELTA;
    }
    if (GetAsyncKeyState('S'))
    {
        cam_pos_x -= sinf(cam_rot) * CAM_SPEED * FRAME_DELTA;
        cam_pos_y -= cosf(cam_rot) * CAM_SPEED * FRAME_DELTA;
    }
    if (GetAsyncKeyState('A'))
        cam_rot -= CAM_SENS * FRAME_DELTA;
    if (GetAsyncKeyState('D'))
        cam_rot += CAM_SENS * FRAME_DELTA;

    if (map_data[(int)cam_pos_y * MAP_SIZE + (int)cam_pos_x] == 1)
    {
        cam_pos_x = last_pos_x;
        cam_pos_y = last_pos_y;
    }
}


void clearScreenBuffer(char character)
{
    memset(screen_buffer, character, screen_buffer_size);

    for (int i = 1; i <= WIN_HEIGHT; i++)
        screen_buffer[(WIN_WIDTH + 1) * i - 1] = '\n';

    //screen_buffer[screen_buffer_size - 1] = '\0';
}


void setPixel(unsigned int pos_x, unsigned int pos_y, char character)
{
    unsigned int index = pos_y * (WIN_WIDTH + 1) + pos_x;

    if (index < screen_buffer_size)
        screen_buffer[index] = character;
}


#if SHOW_MAP
void showMap()
{
    clearScreenBuffer(' ');

    for (unsigned int y = 0; y < MAP_SIZE; y++)
    {
        for (unsigned int x = 0; x < MAP_SIZE; x++)
        {
            char map_val = map_data[y * MAP_SIZE + x];

            if (x == (unsigned int)cam_pos_x && y == (unsigned int)cam_pos_y)
                map_val = 3;

            switch (map_val)
            {
            case 0:
                setPixel(x * 2, y, ' ');
                setPixel(x * 2 + 1, y, ' ');
                break;

            case 1:
                setPixel(x * 2, y, 177);
                setPixel(x * 2 + 1, y, 177);
                break;

            case 2:
                setPixel(x * 2, y, 219);
                setPixel(x * 2 + 1, y, 219);
                break;

            case 3:
                setPixel(x * 2, y, 'P');
                setPixel(x * 2 + 1, y, 'P');
                break;
            
            default:
                break;
            }
        }
    }

    DWORD num_written;
    WriteConsoleA(hConsole, screen_buffer, screen_buffer_size, &num_written, NULL);
}
#endif
