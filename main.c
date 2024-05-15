#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>


#define WIN_WIDTH 108 // 108
#define WIN_HEIGHT 29 // 29

#define FRAME_DELTA 0.016f // 0.016f

#define RAY_STEP_SIZE 0.03f // 0.03f
#define MAX_RAY_STEPS 1000 // 1000

#define CAM_FOV 1.0f // 1.0f
#define CAM_SPEED 1.0f // 1.0f
#define CAM_SENS 4.0f // 4.0f

#define LIGHT_RANGE 10.0f // 10.0f
#define LIGHT_FALLOFF 0.2f // 0.2f

#define MAX_FLOOR_DIST 2.5f // 2.5f
#define MAX_EXIT_DIST 5.0f // 5.0f

#define MAP_SIZE 25 // 25


int screen_buffer_size;
char* screen_buffer;

void clearScreenBuffer(char character);
void setPixel(unsigned int pos_x, unsigned int pos_y, char character);


char* map_data;

const char char_ramp[] = ".:;iX@";

float cam_pos_x = 3.8f;
float cam_pos_y = 3.8f;
float cam_rot = 0.0f;


void generateMap();
void showMap();
void render();
void processInput();


int main() {
    screen_buffer_size = WIN_WIDTH * WIN_HEIGHT + WIN_HEIGHT;
    screen_buffer = malloc(screen_buffer_size);
    if (screen_buffer == NULL) {
        printf("ERROR::FAILED TO ALLOCATE MEMORY\n");
        return -1;
    }

    map_data = malloc(MAP_SIZE * MAP_SIZE);
    if (map_data == NULL) {
        printf("ERROR::FAILED TO ALLOCATE MEMORY\n");
        return -1;
    }
    generateMap();
    showMap();


    while (1) {
        if (*(map_data + (int)cam_pos_y * MAP_SIZE + (int)cam_pos_x) == 2) {
            MessageBox(
                NULL,
                "You escaped the maze!",
                "Congradulations!",
                MB_ICONINFORMATION
            );
            break;
        }


        processInput();
        clearScreenBuffer(32);

        render();

        puts(screen_buffer);
        _sleep(FRAME_DELTA * 1000);
    }


    free(map_data);
    free(screen_buffer);
    return 0;
}


float randomUniform() {
    return (float)rand() / RAND_MAX;
}

void generateMap() {
    srand(time(NULL));
    rand();

    int cur_x = randomUniform() * (MAP_SIZE - 2) + 1;
    int cur_y = 1;

    cam_pos_x = cur_x + 0.5f;
    cam_pos_y = cur_y + 0.5f;

    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++)
        *(map_data + i) = 1;

    *(map_data + cur_y * MAP_SIZE + cur_x) = 0;

    for (int x = 0; x < MAP_SIZE - 2; x++) {
        for (int y = 0; y < MAP_SIZE - 2; y++) {
            if (randomUniform() < 0.666f)
                *(map_data + (y + 1) * MAP_SIZE + x + 1) = 0;
        }
    }

    unsigned int try_count = 0;

    while (1) {
        int test_x = cur_x;
        int test_y = cur_y;

        if (randomUniform() < 0.5f) {
            if (randomUniform() < 0.5f)
                test_x -= 1;
            else
                test_x += 1;
        }
        else {
            if (randomUniform() < 0.5f)
                test_y -= 1;
            else
                test_y += 1;
        }
        
        if (test_x > 0 && test_x < MAP_SIZE - 1 && test_y > 0 && test_y < MAP_SIZE - 1) {
            if (*(map_data + test_y * MAP_SIZE + test_x) == 0 || try_count >= 10) {
                try_count = 0;
                cur_x = test_x;
                cur_y = test_y;
                *(map_data + cur_y * MAP_SIZE + cur_x) = 0;
                
                if (cur_y == MAP_SIZE - 2) {
                    *(map_data + cur_y * MAP_SIZE + cur_x) = 2;
                    break;
                }
            }
            else
                try_count++;
        }
    }
}


void showMap() {
    clearScreenBuffer(32);

    for (int x = 0; x < MAP_SIZE; x++) {
        for (int y = 0; y < MAP_SIZE; y++) {
            int map_val = *(map_data + y * MAP_SIZE + x);

            if ((int)cam_pos_x == x && (int)cam_pos_y == y) {
                setPixel(x * 2, y, 254);
                setPixel(x * 2 + 1, y, 32);
            }
            else if (map_val == 1) {
                setPixel(x * 2, y, 177);
                setPixel(x * 2 + 1, y, 177);
            }
            else if (map_val == 2) {
                setPixel(x * 2, y, 178);
                setPixel(x * 2 + 1, y, 178);
            }
        }
    }

    puts(screen_buffer);
    getchar();
}


void render() {
    for (int i = 0; i < WIN_WIDTH; i++) {
        float ray_rot = cam_rot + ((float)i / WIN_WIDTH - 0.5f) * CAM_FOV;
        float ray_pos_x = cam_pos_x;
        float ray_pos_y = cam_pos_y;
        float ray_dir_x = sinf(ray_rot) * RAY_STEP_SIZE;
        float ray_dir_y = cosf(ray_rot) * RAY_STEP_SIZE;
        float ray_dist = 0.0f;
        float ray_step = RAY_STEP_SIZE * cosf(ray_rot - cam_rot);
        int line_height;
        int line_offset;
        float line_brightness;

        for (int j = 0; j < MAX_RAY_STEPS; j++) {
            float ray_prev_x = ray_pos_x;
            float ray_prev_y = ray_pos_y;
            ray_pos_x += ray_dir_x;
            ray_pos_y += ray_dir_y;
            ray_dist += ray_step;

            if (ray_pos_x < 0.0f || ray_pos_x >= MAP_SIZE || ray_pos_y < 0.0f || ray_pos_y >= MAP_SIZE)
                break;

            if (*(map_data + (int)ray_pos_y * MAP_SIZE + (int)ray_pos_x) == 1)
                break;

            if (((int)ray_prev_x != (int)ray_pos_x || (int)ray_prev_y != (int)ray_pos_y) && ray_dist < MAX_FLOOR_DIST) {
                line_height = (1.0f / ray_dist) * WIN_HEIGHT;
                if (line_height > WIN_HEIGHT) continue;

                line_offset = (WIN_HEIGHT - line_height) / 2;

                line_brightness = LIGHT_FALLOFF / (LIGHT_FALLOFF + ray_dist * ray_dist * (1.0f / LIGHT_RANGE));

                setPixel(i, (line_height - 1) + line_offset, char_ramp[(int)(line_brightness * (sizeof(char_ramp) - 1))]);
            }

            if (*(map_data + (int)ray_pos_y * MAP_SIZE + (int)ray_pos_x) == 2 && ray_dist < MAX_EXIT_DIST) {
                line_height = (1.0f / ray_dist) * WIN_HEIGHT;
                if (line_height > WIN_HEIGHT) continue;

                line_offset = (WIN_HEIGHT - line_height) / 2;

                setPixel(i, (line_height - 1) + line_offset, '@');
            }
        }

        line_height = min((1.0f / ray_dist) * WIN_HEIGHT, WIN_HEIGHT);
        line_offset = (WIN_HEIGHT - line_height) / 2;

        line_brightness = LIGHT_FALLOFF / (LIGHT_FALLOFF + ray_dist * ray_dist * (1.0f / LIGHT_RANGE));

        for (int j = 0; j < line_height; j++) {
            setPixel(i, j + line_offset, char_ramp[(int)(line_brightness * (sizeof(char_ramp) - 1))]);
        }
    }
}


void processInput() {
    float last_pos_x = cam_pos_x;
    float last_pos_y = cam_pos_y;

    if (GetAsyncKeyState('W')) {
        cam_pos_x += sinf(cam_rot) * CAM_SPEED * FRAME_DELTA;
        cam_pos_y += cosf(cam_rot) * CAM_SPEED * FRAME_DELTA;
    }
    if (GetAsyncKeyState('S')) {
        cam_pos_x -= sinf(cam_rot) * CAM_SPEED * FRAME_DELTA;
        cam_pos_y -= cosf(cam_rot) * CAM_SPEED * FRAME_DELTA;
    }
    if (GetAsyncKeyState('A'))
        cam_rot -= CAM_SENS * FRAME_DELTA;
    if (GetAsyncKeyState('D'))
        cam_rot += CAM_SENS * FRAME_DELTA;

    if (*(map_data + (int)cam_pos_y * MAP_SIZE + (int)cam_pos_x) == 1) {
        cam_pos_x = last_pos_x;
        cam_pos_y = last_pos_y;
    }
}


void clearScreenBuffer(char character) {
    for (int i = 0; i < screen_buffer_size; i++)
        *(screen_buffer + i) = character;

    for (int i = 1; i <= WIN_HEIGHT; i++)
        *(screen_buffer + (WIN_WIDTH + 1) * i - 1) = 10;

    *(screen_buffer + screen_buffer_size - 1) = 0;
}

void setPixel(unsigned int pos_x, unsigned int pos_y, char character) {
    int index = pos_y * (WIN_WIDTH + 1) + pos_x;
    if (index < screen_buffer_size)
        *(screen_buffer + index) = character;
    else
        printf("ERROR: INVALID PIXEL COORDINATE\n");
}