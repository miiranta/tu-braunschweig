struct Animate_Params
{
    int x;
    int y;
    int ms_per_step;
    int TTL = 0;
};

struct Animate_Params_Sprite
{
    string key;

    string sprite;

    float alpha = 1;
    float r_blend = 255;
    float g_blend = 255;
    float b_blend = 255;

    float x = 0;
    float y = 0;

    int num_frames = 0;

    int ms_per_step = 0;

    long long int starting_ms = 0;    
    int last_frame = 0;
    long long int last_frame_ms = 0;

    float scale = 1;

    int self_destruct_after_iterations = -1;
    int ms_start_delay = 0;
};
