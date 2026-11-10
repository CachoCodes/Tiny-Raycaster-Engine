#include <algorithm> //fill_n()
#include <cmath> //sen() y cos()
#include <cstdint>

int win_height = 720;
int win_width = 1280;
bool debug_mode = false;
uint32_t *framebuffer = nullptr;

int player_x = 800;
int player_y = 800;
int speed = 200;
int looking_at = 0;

namespace raycaster{
    bool w_pressed = false;
    bool s_pressed = false;
    bool d_pressed = false;
    bool a_pressed = false;
    bool left_pressed = false;
    bool right_pressed = false;
    bool shift_pressed = false;
    
    const int definition = 128;
    const int FOV = 90;    
    int section_width = static_cast<int>(win_width / definition);
    int sections[definition];
    int sections_color[definition];
    float separation_degrees = static_cast<float>(FOV) / definition;

    //1 = Blanco
    //2 = Verde
    //3 = Rojo
    //4 = Azul
    //5 = Amarillo
    int grid[16][16] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1},
        {1,0,0,0,3,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,4,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,5,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    void clear_framebuffer(uint32_t *framebuffer, uint32_t color){
        std::fill_n(framebuffer, win_width * win_height, color);
    }

    void set_pixel(uint32_t *framebuffer, int x, int y, uint32_t color){
        if (x >= 0 && x < win_width && y >= 0 && y < win_height) {
            framebuffer[y * win_width + x] = color;
        }
    }

    void draw_rect(uint32_t *framebuffer, int x1, int y1, int x2, int y2, uint32_t color) {
        int start_x = std::min(x1, x2);
        int end_x   = std::max(x1, x2);
        int start_y = std::min(y1, y2);
        int end_y   = std::max(y1, y2);

        for (int y = start_y; y < end_y; y++) {
            for (int x = start_x; x < end_x; x++) {
                set_pixel(framebuffer, x, y, color);
            }
        }
    }

    int precalc_of_areas[16][16][2][2];
}

void precalculate_areas(){
    for (int x = 0; x <= 15; x++){
        for (int y = 0; y <= 15; y++){
            int current_val = raycaster::grid[x][y];
            if (current_val == 0) continue;
            //Higher X
            raycaster::precalc_of_areas[x][y][0][0] = ((x + 1) * 100) + 50;
            //Higher Y
            raycaster::precalc_of_areas[x][y][0][1] = ((y + 1) * 100) + 50;
            //Lower X
            raycaster::precalc_of_areas[x][y][1][0] = ((x + 1) * 100) - 50;
            //Lower Y
            raycaster::precalc_of_areas[x][y][1][1] = ((y + 1) * 100) - 50;
        }
    }
}

void calculate_rays(){
    for(int i = 0; i < raycaster::definition; i++){
        float current_degree = (i * raycaster::separation_degrees) * (3.14159265f / 180.0f) + looking_at * (3.14159265f / 180.0f);
        float x_step = std::cosf(current_degree);
        float y_step = std::sinf(current_degree);
        float current_x = player_x;
        float current_y = player_y;
        bool intersected = false;
        int distance = 0;

        while(!intersected){
            distance++;
            current_x += x_step * 5;
            current_y += y_step * 5;
            int color = 0;
            for(int x = 0; x <= 15 && !intersected; x++){
                for(int y = 0; y <= 15; y++){
                    if(raycaster::grid[x][y] == 0) continue;
                    color = raycaster::grid[x][y];
                    int h_x = raycaster::precalc_of_areas[x][y][0][0];
                    int h_y = raycaster::precalc_of_areas[x][y][0][1];
                    int l_x = raycaster::precalc_of_areas[x][y][1][0];
                    int l_y = raycaster::precalc_of_areas[x][y][1][1];
                    
                    // ABAB shit
                    bool x_coincidence = h_x >= current_x && l_x <= current_x;
                    bool y_coincidence = h_y >= current_y && l_y <= current_y;
                    bool is_inside = x_coincidence && y_coincidence;
                    raycaster::sections_color[i] = color;
                    
                    if(is_inside) {
                        intersected = true; 
                        break;              
                    }
                }
            }
            if (distance > 400) { 
                distance = 9999;
                intersected = true;
                raycaster::sections[i] = distance;
                raycaster::sections_color[i] = color;
                break; 
            }
        raycaster::sections[i] = distance;
    }
    }
}

void draw_sections(){
    for (int i = 0; i < raycaster::definition; i++) {
        int distance = raycaster::sections[i];
        if (distance == 9999) continue;
        if (distance <= 0) distance = 1;
        int wall_height = (win_height * 40) / distance;
        int start_y = (win_height - wall_height) / 2;
        int end_y = start_y + wall_height;

        int x1 = i * raycaster::section_width;
        int x2 = x1 + raycaster::section_width;
        //1 = Blanco
        //2 = Verde
        //3 = Rojo
        //4 = Azul
        //5 = Amarillo
        if (raycaster::sections_color[i] == 1)
            raycaster::draw_rect(framebuffer, x1, start_y, x2, end_y, 0x00FFFFFF);
        
        if (raycaster::sections_color[i] == 2)
            raycaster::draw_rect(framebuffer, x1, start_y, x2, end_y, 0x0000FF00);
            
        if (raycaster::sections_color[i] == 3)
            raycaster::draw_rect(framebuffer, x1, start_y, x2, end_y, 0x00FF0000);
            
        if (raycaster::sections_color[i] == 4)
            raycaster::draw_rect(framebuffer, x1, start_y, x2, end_y, 0x000000FF);
            
        if (raycaster::sections_color[i] == 5)
            raycaster::draw_rect(framebuffer, x1, start_y, x2, end_y, 0x00FFFF00);
    }
}

void init_render(){
    framebuffer = new uint32_t[win_width * win_height];
    precalculate_areas();
}

void input_loop(){
    float direction_x = std::cosf((looking_at + raycaster::FOV/2) * (3.14159265f / 180.0f));
    float direction_y = std::sinf((looking_at + raycaster::FOV/2) * (3.14159265f / 180.0f));

    speed = (raycaster::shift_pressed) ? 400 : 200;

    if(raycaster::w_pressed){
        player_x += static_cast<int>(direction_x * speed * delta);
        player_y += static_cast<int>(direction_y * speed * delta);
    }
    if(raycaster::s_pressed){
        player_x -= static_cast<int>(direction_x * speed * delta);
        player_y -= static_cast<int>(direction_y * speed * delta);
    }
    if(raycaster::a_pressed){
        player_x += static_cast<int>(direction_y * speed * delta);
        player_y -= static_cast<int>(direction_x * speed * delta);
    }
    if(raycaster::d_pressed){
        player_x -= static_cast<int>(direction_y * speed * delta);
        player_y += static_cast<int>(direction_x * speed * delta);
    }
    if(raycaster::left_pressed)
        looking_at -= 180 * delta;
    if(raycaster::right_pressed)
        looking_at += 180 * delta;
}

void render_loop(uint32_t *framebuffer){
    raycaster::clear_framebuffer(framebuffer, 0x00000000);
    raycaster::draw_rect(framebuffer,0,win_height/2,win_width,0, 0x00999999);
    calculate_rays();
    draw_sections();
}