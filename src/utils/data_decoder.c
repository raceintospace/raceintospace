#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

void print_escaped_string(const char * string, int max_length)
{
    int i;
    for (i = 0; i < max_length; i ++) {
        char c = string[i];
        switch (c) {
            case 0:
                // NUL = end-of-string
                return;
            
            case '\\': fwrite("\\\\", 1, 4, stdout); break;
            case '"': fwrite("\\\"", 1, 4, stdout); break;

            default:
                if (c >= ' ' && c <= '~')
                    putc(c, stdout);
                else
                    printf("\\x%02x", c);
        }
    }
}

#define RecordStart() { printf("  {\n"); }
#define RecordEnd() { printf("  },\n"); }
#define Number(name) { printf("    ." #name " = %i,\n", (int)record.name); }
#define NumberArray(name) do { \
        int i; \
        printf("    ." #name " = {"); \
        for (i = 0; i < (sizeof(record.name) / sizeof(record.name[0])); i++) { \
            printf("%s %i", (i == 0 ? "" : ","), (int)record.name[i]); \
        } \
        printf(" },\n"); \
    } while(0)
#define String(name) { \
    printf("    ." #name " = \""); \
    print_escaped_string(record.name, sizeof(record.name)); \
    printf("\",\n", record.name); \
    }

void write_mission(FILE * fp)
{
    struct {
        char Index;       /**< Mission Index Number */
        char Name[50];    /**< Name of Mission */
        char Abbr[24];    /**< Name of Mission Abbreviated */
        char Code[62];    /**< Mission Coding String */
        char Alt[36];     /**< Alternate Sequence */
        char AltD[36];    /**< Alternate Sequence for Deaths (Jt Missions Only) */
        char Days,        /**< Duration Level */
             Dur,              /**< Duration Mission */
             Doc,              /**< Docking Mission */
             EVA,              /**< EVA Mission */
             LM,               /**< LM Mission */
             Jt,               /**< Joint Mission */
             Lun,              /**< Lunar Mission */
             mEq,              /**< Minimum Equipment */
             mCrew;            /**< Useful for Morgans Code */
        uint8_t mVab[2];  /**< Hardware Requirements */
        char PCat[5],     /**< Prestige Category List */
             LMAd;             /**< LM Addition Points */
    } __attribute__((packed)) record;

    printf("struct mStr missions[] = {\n");
    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        Number(Index);
        String(Name);
        String(Abbr);
        String(Code);
        NumberArray(Alt);
        NumberArray(AltD);
        Number(Days);
        Number(Dur);
        Number(Doc);
        Number(EVA);
        Number(LM);
        Number(Jt);
        Number(Lun);
        Number(mEq);
        Number(mCrew);
        NumberArray(mVab);
        NumberArray(PCat);
        Number(LMAd);
        RecordEnd();
    }
    printf("};\n\n");
}

void write_men(FILE * fp, const char * name)
{
    struct ManPool {
        char Name[14], Sex, Cap, LM, EVA, Docking, Endurance;
    } __attribute__((packed)) record;
    
    printf("struct ManPool %s[] = {\n", name);
    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        String(Name);
        Number(Sex);
        Number(Cap);
        Number(LM);
        Number(EVA);
        Number(Docking);
        Number(Endurance);
        RecordEnd();
    }
    printf("};\n\n");
}

void write_historical_men(FILE * fp) {
    write_men(fp, "historical_men");
}

void write_custom_men(FILE * fp) {
    write_men(fp, "custom_men");
}

void write_events(FILE * fp)
{
    int i;
    struct event {
        int player;
        int bud;        // this is the parameter to OpenNews(), but I don't know what it means
        char description[250];
    } record;
    
    printf("struct event_t events[] = {\n");
    while (fread(&record.description, sizeof(record.description), 1, fp)) {
        record.player = i % 2;
        record.bud = i / 2;
        RecordStart();
        Number(player);
        Number(bud);
        String(description);
        RecordEnd();

        i ++;
    }
    printf("};\n\n");
}

void write_news(FILE * fp)
{
    int i;
    struct {
        uint32_t us_news;
        uint32_t soviet_news;
        uint32_t unknown[3];
    }  __attribute__((packed)) lengths;
    
    struct {
        uint32_t us_news;
        uint32_t soviet_news;
    } offsets;

    struct {
        int i;
        int player;
        int year;
        int season;
        char description[256];
    } record;
    
    // the file contains lengths, and we want offsets
    fread(&lengths, sizeof(lengths), 1, fp);
    offsets.us_news = sizeof(lengths);
    offsets.soviet_news = offsets.us_news + lengths.us_news;
    
    printf("struct news_t news[] = {\n");
    
    fseek(fp, offsets.us_news, SEEK_SET);
    record.player = 0;
    for (i = 0; i <= 122; i++) {
        fread(record.description, 232, 1, fp);

        // i = ((Data->Year - 57) * 6 + Data->Season * 3 + random(3))
        record.i = i;
        record.season = (i / 3) % 2;
        record.year = i / 6 + 1957;
        
        RecordStart();
        Number(i);
        Number(year);
        Number(season);
        Number(player);
        String(description);
        RecordEnd();
    }
    
    fseek(fp, offsets.soviet_news, SEEK_SET);
    record.player = 1;
    for (i = 0; i <= 122; i++) {
        fread(record.description, 177, 1, fp);
        
        // i = ((Data->Year - 57) * 4 + Data->Season * 2 + random(2))
        record.i = i;
        record.season = (i / 2) % 2;
        record.year = i / 4 + 1957;
        
        RecordStart();
        Number(i);
        Number(year);
        Number(season);
        Number(player);
        String(description);
        RecordEnd();
    }
    
    printf("};\n\n");
}


void write_failure_modes(FILE * fp)
{
    int i;
    uint32_t count;  // number of failure modes

    struct FailureHeaderStruct {
        char Code[6];
        int32_t offset;
        int16_t size;
    } failsHdr[44];

    struct XFails {
        int32_t percentage;
        int16_t code,value,extra;
        int16_t fail;  // Failure value
        char description[200];
    } record;

    fread(&count, 4, 1, fp);
    
    memset(&failsHdr,0, sizeof(failsHdr));
    
    for (i = 0; i < count; i++) {
        fread(failsHdr[i].Code, 6, 1, fp);
        fread(&failsHdr[i].offset, 4, 1, fp);
        fread(&failsHdr[i].size, 2, 1, fp);
    }
    
    // Read each failure sequence
    for (i = 0; i < count; i++) {
        size_t sz = failsHdr[i].size;
        
        fseek(fp, failsHdr[i].offset, SEEK_SET);
        printf("struct failuretext_t failuretext_%s[] = {\n", failsHdr[i].Code);

        while (sz) {
            fread(&record.percentage, 4, 1, fp);
            fread(&record.code, 2, 1, fp);
            fread(&record.value, 2, 1, fp);
            fread(&record.extra, 2, 1, fp);
            fread(&record.fail, 2, 1, fp);
            fread(&record.description, 200, 1, fp);

            //fread(&record, 208, 1, fp);
            RecordStart();
            Number(percentage);
            Number(code);
            Number(value);
            Number(extra);
            Number(fail);
            String(description);
            RecordEnd();
            sz -= 212;
        }
        printf("};\n\n");

    }

}

void write_animation_key(FILE * fp, const char * name)
{
    // These are the filenames of the animations.  The SEQ.DAT and FSEQ.DAT reference these by index
    uint16_t count;
    struct SeqKey {
        int Index;
        char Sequence[8];
    }record; 
    
    record.Index = 0;
    fread(&count, 2, 1, fp);  // Initial 16-bit value is the count
    printf("struct MissionSequenceKey %s[] = {\n", name);
    while (fread(&record.Sequence, 8, 1, fp)) {
        RecordStart();
        Number(Index);
        String(Sequence);
        RecordEnd();
        record.Index++;
    }
    printf("};\n\n");
}


void write_mission_animation_success_key(FILE * fp) {
    write_animation_key(fp, "success_animation_key");
}

void write_mission_animation_failure_key(FILE * fp) {
    write_animation_key(fp, "failure_animation_key");
}

void decode(const char * dir, const char * filename, void(*function)(FILE *))
{
    char full_path[512];
    FILE * fp;
    
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, filename);
    fp = fopen(full_path, "rb");

    if (!fp) {
        char error[600];
        snprintf(error, sizeof(error), "unable to open %s", full_path);
        perror(error);
        exit(1);
    }
    
    function(fp);
    
    fclose(fp);
}

int main(int argc, char ** argv)
{
    FILE * fp;
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s path/to/gamedata\n", argv[0]);
        return 1;
    }
    
    decode(argv[1], "mission.dat", write_mission);
    
    decode(argv[1], "crew.dat", write_historical_men);
    decode(argv[1], "user.dat", write_custom_men);
    
    decode(argv[1], "event.dat", write_events);

    decode(argv[1], "news.dat", write_news);
    
    decode(argv[1], "fails.cdr", write_failure_modes);
    
    decode(argv[1], "seq.key", write_mission_animation_success_key);
    decode(argv[1], "fseq.key", write_mission_animation_failure_key);
    //decode(argv[1], "seq.dat", write_mission_success_sequence);
    //decode(argv[1], "fseq.dat", write_mission_failure_sequence);
    
    return 0;
}
