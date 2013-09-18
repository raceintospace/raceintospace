
int game_main(int argc, char *argv[]);

int main(int argc, char **argv)
{
    if (!SDL_getenv("SDL_AUDIODRIVER")) {
        SDL_putenv("SDL_AUDIODRIVER=waveout");
    }

    return game_main(argc, argv);
}
