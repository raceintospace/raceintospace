#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <jsoncpp/json/json.h>
#include <assert.h>

void print_escaped_string(const char *string, int max_length)
{
    int i;

    for (i = 0; i < max_length; i ++) {
        char c = string[i];

        switch (c) {
        case 0:
            // NUL = end-of-string
            return;

        case '\\':
            fwrite("\\\\", 1, 4, stdout);
            break;

        case '"':
            fwrite("\\\"", 1, 4, stdout);
            break;

        default:
            if (c >= ' ' && c <= '~') {
                putc(c, stdout);
            } else {
                printf("\\x%02x", c);
            }
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
    printf("\",\n"); \
    }




void write_mission(FILE *fp)
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

int
RLED(void *src_raw, void *dest_raw, unsigned int src_size)
{
    signed char *src = (signed char *)src_raw;
    signed char *dest = (signed char *)dest_raw;
    unsigned int used;
    int count, val;
    int i;

    used = 0;

    while (used < src_size) {
        count = src[used++];

        if (count < 0) {
            count = -count + 1;
            val = src[used++];

            for (i = 0; i < count; i++) {
                *dest++ = val;
            }
        } else {
            count++;

            for (i = 0; i < count; i++) {
                *dest++ = src[used++];
            }
        }
    }

    return ((char *)dest - (char *)dest_raw);
}

void write_men(FILE *fp, const char *name)
{
    int count = 0;
    struct ManPool {
        char Name[14], Sex, Cap, LM, EVA, Docking, Endurance;
    } __attribute__((packed)) record;

    // magic hardcoded group data
    struct group_t {
        int entries, delay, number;
    } groups[] = {
        { 11 + 3, 6,  7 },
        { 18 + 3, 4,  9 },
        { 20 + 3, 4,  14 },
        { 28,     8,  16 },
        { 20,     99, 14 },
    };
    const int groups_size = 5;

    Json::Value json(Json::arrayValue);

    for (int player = 0; player < 2; player++) {
        Json::Value json_player(Json::objectValue);
        json_player["player"] = player;

        Json::Value json_groups(Json::arrayValue);

        for (int group_number = 0; group_number < groups_size; group_number++) {
            group_t &group = groups[group_number];

            Json::Value json_group(Json::objectValue);
            json_group["recruiting_delay"] = group.delay;
            json_group["number_to_choose"] = group.number;

            Json::Value json_entries(Json::arrayValue);

            for (int i = 0; i < group.entries; i++) {
                ssize_t records = fread(&record, sizeof(record), 1, fp);
                assert(records == 1);

                Json::Value json_entry(Json::objectValue);
                json_entry["endurance"] = record.Endurance;
                json_entry["docking"] = record.Docking;
                json_entry["eva"] = record.EVA;
                json_entry["lunar"] = record.LM;
                json_entry["capsule"] = record.Cap;

                if (record.Sex == 1) {
                    json_entry["female"] = true;
                }

                json_entry["name"] = record.Name;
                json_entries.append(json_entry);
            }

            json_group["entries"] = json_entries;
            json_groups.append(json_group);
        }

        json_player["groups"] = json_groups;
        json.append(json_player);
    }

    // ensure we're at EOF
    assert(fread(&record, sizeof(record), 1, fp) < 1);

    std::cout << json << std::endl << std::endl;

    exit(0);
}

void write_historical_men(FILE *fp)
{
    write_men(fp, "historical_men");
}

void write_custom_men(FILE *fp)
{
    write_men(fp, "custom_men");
}

void write_events(FILE *fp)
{
    int i = 0;
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

void write_news(FILE *fp)
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
#pragma mark

// End of game message text
void write_endgame(FILE *fp)
{
    // 600 byte blocks
    struct EndGameText {
        char Text[600];
    } __attribute__((packed)) record;

    printf("struct endgame_t endgame[] = {\n");

    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        String(Text);
        RecordEnd();
    }

    printf("};\n\n");
}

void write_post_mission_review(FILE *fp)
{
    int count = 0;
    struct MissionReview {
        char Text[204];
    } __attribute__((packed)) record;

    printf("struct mission_review_t mission_review[] = {\n");

    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        printf("    .Country = \"%s\"\n", (count < 18) ? "US" : "SOV");
        String(Text);
        RecordEnd();
        count++;
    }

    printf("};\n\n");

}

void write_records(FILE *fp)
{
    // This is where user records are stored, it will be created
    struct  {
        char country;
        char month;
        char yr;
        char program;
        int16_t  tag;
        char type;
        char place;
        char name[20];
        char astro[14];
    } record;

    // TODO: represent this in a better way
    printf("struct vab_drawing_offsets_t vab_drawing_offsets[] = {\n");

    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        Number(country);
        Number(month);
        Number(yr);
        Number(program);
        Number(tag);
        Number(type);
        Number(place);
        String(name);
        String(astro);
        RecordEnd();
    }

    printf("};\n\n");
}

void write_vab_drawing_offsets(FILE *fp)
{
    struct VabDrawingOffsets {
        int16_t x1, y1, x2, y2, o;
    } record;

    // TODO: represent this in a better way
    printf("struct vab_drawing_offsets_t vab_drawing_offsets[] = {\n");

    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        Number(x1);
        Number(y1);
        Number(x2);
        Number(y2);
        Number(o);
        RecordEnd();
    }

    printf("};\n\n");
}



void write_budget_mods(FILE *fp)
{
    int level, modifier, i;

    struct {
        int16_t modifiers[10];
    } record;

    // [3=levels] [6] [10]

    for (level = 0; level < 3; level++) {
        printf("struct budget_mods_t budget_mods_Level_%d[6] = {\n", level + 1);

        for (modifier = 0; modifier < 6; modifier++) {
            fread(&record, sizeof(record), 1, fp);
            printf("    .modifiers[%d] = {", modifier);

            for (i = 0; i < 10; i++)  {
                printf("%d%s", record.modifiers[i], (i != 9) ? ", " : "");
            }

            printf(" },\n");
        }

        printf("};\n\n");
    }
}

#include "../game/data.h"

void write_player_data(FILE *fp)
{
    char buffer[4096];
    size_t bytes_read = 0;
    size_t bytes_uncompressed = 0;

    struct {
        struct Players players[2];
    } record;


    bytes_read = fread(buffer, 1, 4096, fp);
    bytes_uncompressed = RLED(buffer, &record, bytes_read);

//#define WRITE_UNCOMPRESSED_RAST_DAT_FILE
#ifdef WRITE_UNCOMPRESSED_RAST_DAT_FILE
    FILE *fout = NULL;
    fout = fopen("URAST.DAT", "wb");
    fwrite(&record, bytes_uncompressed, 1, fout);
    fclose(fout);
#endif
    // TODO: represent this in a better way
    printf("struct players_t players[] = {\n");

    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        //String(players[0].BUZZ);
        //String(players[1].BUZZ);
        RecordEnd();
    }

    printf("};\n\n");
}

void write_historical_equip(FILE *fp)
{
    /*
    fread(&Data->P[0].Probe[0], 28 * (sizeof(Equipment)), 1, fin);
    fread(&Data->P[1].Probe[0], 28 * (sizeof(Equipment)), 1, fin);
     */

    struct Equipment {
        char Name[20];      /**< Name of Hardware */
        char ID[2];         /**< EquipID "C0 C1 C2 C3 : Acts as Index */
        int16_t Safety;     /**< current safety factor */
        int16_t MisSaf;     /**< Safety During Mission */
        int16_t MSF;        /**< used only to hold safety for docking kludge / Base Max R&D for others (from 1.0.0)*/
        char Base;          /**< initial safety factor */
        int16_t InitCost;   /**< Startup Cost of Unit */
        char UnitCost;      /**< Individual Cost */
        int16_t UnitWeight; /**< Weight of the Item */
        int16_t MaxPay;     /**< Maximum payload */
        char RDCost;        /**< Cost of R&D per roll */
        char Code;          /**< Equipment Code for qty scheduled */
        char Num;           /**< quantity in inventory */
        char Spok;          /**< qty being used on missions */
        char Seas;          /**< Seasons Program is Active */
        char Used;          /**< total number used in space */
        char IDX[2];        /**< EquipID "C0 C1 C2 C3 : Acts as Index */
        int16_t Steps;      /**< Program Steps Used */
        int16_t Failures;   /**< number of program failures */
        char MaxRD;         /**< maximum R & D */
        char MaxSafety;     /**< maximum safety factor */
        char SMods;         /**< safety factor mods for next launch */
        char SaveCard;      /**< counter next failure in this prog */
        char Delay;         /**< delay in purchase - in seasons */
        char Duration;      /**< Days it can last in space */
        char Damage;        /**< Damage percent for next launch */
        char DCost;         /**< Cost to repair damage */
        char MisSucc;       /**< Mission Successes */
        char MisFail;       /**< Mission Failures */
    } __attribute__((packed)) record;

    printf("struct equipment_t historical_equipment[] = {\n");

    // Some of these values shouldn't be read in as they are for record keeping
    while (fread(&record, sizeof(record), 1, fp)) {
        RecordStart();
        String(Name);
        printf("    .ID[2] = '%c%c'\n", record.ID[0], record.ID[1]);
        Number(Safety);
        Number(MisSaf);
        Number(MSF);
        Number(Base);
        Number(InitCost);
        Number(UnitCost);
        Number(UnitWeight);
        Number(MaxPay);
        Number(RDCost);
        Number(Code);
        Number(Num);
        Number(Spok);
        Number(Seas);
        Number(Used);
        printf("    .IDX[2] = '%c%c'\n", record.IDX[0], record.IDX[1]);
        Number(Steps);
        Number(Failures);
        Number(MaxRD);
        Number(MaxSafety);
        Number(SMods);
        Number(SaveCard);
        Number(Delay);
        Number(Duration);
        Number(Damage);
        Number(DCost);
        Number(MisSucc);
        Number(MisFail);

        RecordEnd();
    }

    printf("};\n\n");
}



#pragma mark

void write_help(FILE *fp)
{
    int i;
    char buffer[2048];  // Covers the largest helptext size

    struct HelpTextHdr {
        char Code[6];
        uint32_t offset;
        uint16_t size;
    } helpHdr[200];

    struct {
        char Code[6];
        int index;
        char description[100][40];
    } record;

    uint32_t count = 0;
    fread(&count, 4, 1, fp);

    // First is a table with offsets to the data
    for (i = 0; i < count; i++) {
        fread(helpHdr[i].Code, 6, 1, fp);
        fread(&helpHdr[i].offset, 4, 1, fp);
        fread(&helpHdr[i].size, 2, 1, fp);
    }

    printf("struct helptext_t helptext[] = {\n");

    // Now just description blobs of text.
    for (i = 0; i < count; i++) {
        char *token;
        int n = 0;
        int j;

        memset(buffer, 0, 2048);
        fseek(fp, helpHdr[i].offset, SEEK_SET);
        fread(buffer, helpHdr[i].size, 1, fp);

        strncpy(record.Code, helpHdr[i].Code, 6);
        token = strtok(buffer, "\n\r");

        while (token != NULL && n < 100) {
            strncpy(record.description[n++], token, 40);
            token = strtok(NULL, "\n\r");
        }

        RecordStart();
        String(Code);
        Number(index);
        printf("    .description[][] = {\n");

        for (j = 0; j < n; j++)  {
            printf("      \"%s\",\n", record.description[j]);
        }

        printf("    },\n");
        RecordEnd();
    }

    printf("};\n\n");
}


void write_failure_modes(FILE *fp)
{
    int i;
    uint32_t count;  // number of failure modes

    struct FailureHeaderStruct {
        char MissionStep[6];
        int32_t offset;
        int16_t size;
    } failsHdr[44];

    struct XFails {
        char MissionStep[6];
        int32_t percentage;
        int16_t code, value, extra;
        int16_t fail;  // Failure value
        char description[200];
    } record;

    fread(&count, 4, 1, fp);

    memset(&failsHdr, 0, sizeof(failsHdr));

    for (i = 0; i < count; i++) {
        fread(failsHdr[i].MissionStep, 6, 1, fp);
        fread(&failsHdr[i].offset, 4, 1, fp);
        fread(&failsHdr[i].size, 2, 1, fp);
    }

    // Read each failure sequence
    printf("struct failuretext_t failuretext[] = {\n");

    for (i = 0; i < count; i++) {
        size_t sz = failsHdr[i].size;
        strcpy(record.MissionStep, failsHdr[i].MissionStep);

        fseek(fp, failsHdr[i].offset, SEEK_SET);

        while (sz) {
            fread(&record.percentage, 4, 1, fp);
            fread(&record.code, 2, 1, fp);
            fread(&record.value, 2, 1, fp);
            fread(&record.extra, 2, 1, fp);
            fread(&record.fail, 2, 1, fp);
            fread(&record.description, 200, 1, fp);

            //fread(&record, 208, 1, fp);
            RecordStart();
            String(MissionStep);
            Number(percentage);
            Number(code);
            Number(value);
            Number(extra);
            Number(fail);
            String(description);
            RecordEnd();
            sz -= 212;
        }

    }

    printf("};\n\n");

}


void write_animation_sequence(FILE *fp, FILE *fp2, const char *name)
{
    // These are the filenames of the animations.  The SEQ.DAT and FSEQ.DAT reference these by index
    struct Sequence {
        char MissionIdSequence[16];    // 10 for SEQ.DAT, 15 for FSEQ.DAT
        char Step[6];   // The step for the failures

        int16_t avIndex[10];

        // using the key file index
        char video[5][10];
        char audio[5][10];
    } record;

    // Read index of animation sequences
    struct SeqKey {
        char Sequence[8];
    } keys[1000];

    int index = 0;
    uint16_t count;
    fread(&count, 2, 1, fp2);  // Initial 16-bit value is the count

    while (fread(&keys[index].Sequence, 8, count, fp2));

    memset(&record, 0, sizeof(record));

    printf("struct MissionSequenceKey %s[] = {\n", name);

    while (fread(&record.MissionIdSequence, 10, 1, fp)) {
        fread(&record.avIndex, 10, 2, fp);

        RecordStart();
        String(MissionIdSequence);
        printf("    .video[] = { \"%s\", \"%s\", \"%s\",\"%s\",\"%s\" },\n",
               keys[record.avIndex[0]].Sequence, keys[record.avIndex[2]].Sequence,
               keys[record.avIndex[4]].Sequence, keys[record.avIndex[6]].Sequence,
               keys[record.avIndex[8]].Sequence);
        printf("    .audio[] = { \"%s\", \"%s\", \"%s\",\"%s\",\"%s\" },\n",
               keys[record.avIndex[1]].Sequence, keys[record.avIndex[2]].Sequence,
               keys[record.avIndex[5]].Sequence, keys[record.avIndex[7]].Sequence,
               keys[record.avIndex[9]].Sequence);
        RecordEnd();
        memset(&record, 0, sizeof(record));
    }

    printf("};\n\n");
}

void write_animation_fsequence(FILE *fp, FILE *fp2, const char *name)
{
    int i;
    // These are the filenames of the animations.  The SEQ.DAT and FSEQ.DAT reference these by index

    struct Table {
        char MissionStep[8];
        int32_t foffset;
        uint16_t size;
    } offsetTable[50];


    struct Sequence {
        char MissionIdSequence[16];    // 10 for SEQ.DAT, 15 for FSEQ.DAT
        char MissionStep[6];   // The step for the failures

        int16_t avIndex[10];

        // using the key file index
        char video[5][10];
        char audio[5][10];
    } record;


    // Read index of animation sequences
    struct SeqKey {
        char Sequence[8];
    } keys[1000];


    for (i = 0; i < 46; i++) {
        fread(&offsetTable[i].MissionStep, 8, 1, fp);
        fread(&offsetTable[i].foffset, 4, 1, fp);
        fread(&offsetTable[i].size, 2, 1, fp);
    }

    // Read the keyfile
    uint16_t count;
    fread(&count, 2, 1, fp2);  // Initial 16-bit value is the count

    while (fread(keys, 8, count, fp2));


    for (i = 0; i < 46; i++) {

        fseek(fp, offsetTable[i].foffset, SEEK_SET);
        memset(&record, 0, sizeof(record));

        printf("struct MissionSequenceKey %s[] = {\n", name);
        int size = offsetTable[i].size;

        while (size) {
            fread(&record.MissionIdSequence, 15, 1, fp); // read name
            fread(&record.avIndex, 10, 2, fp);
            strcpy(record.MissionStep, offsetTable[i].MissionStep);

            size -= (15 + 10 * 2); // string + 5 ui16 pairs

            RecordStart();
            String(MissionStep);
            String(MissionIdSequence);
            printf("    .video[] = { \"%s\", \"%s\", \"%s\",\"%s\",\"%s\" },\n",
                   keys[record.avIndex[0]].Sequence, keys[record.avIndex[2]].Sequence,
                   keys[record.avIndex[4]].Sequence, keys[record.avIndex[6]].Sequence,
                   keys[record.avIndex[8]].Sequence);
            printf("    .audio[] = { \"%s\", \"%s\", \"%s\",\"%s\",\"%s\" },\n",
                   keys[record.avIndex[1]].Sequence, keys[record.avIndex[2]].Sequence,
                   keys[record.avIndex[5]].Sequence, keys[record.avIndex[7]].Sequence,
                   keys[record.avIndex[9]].Sequence);
            RecordEnd();
            memset(&record, 0, sizeof(record));
        }
    }

    printf("};\n\n");
}


void write_mission_animation_success_sequence(FILE *fp, FILE *fp2)
{
    write_animation_sequence(fp, fp2, "success_animation_sequence");
}

void write_mission_animation_failure_sequence(FILE *fp, FILE *fp2)
{
    write_animation_fsequence(fp, fp2, "failure_animation_sequence");
}


// Decode mission sequence files.  This requires a data file and a keyfile
void decode_seq(const char *dir, const char *filename, void(*function)(FILE *, FILE *))
{
    char full_seq_path[512];
    char full_key_path[512];

    FILE *fp;
    FILE *fp2;


    snprintf(full_seq_path, sizeof(full_seq_path), "%s/%s.dat", dir, filename);
    fp = fopen(full_seq_path, "rb");

    if (!fp) {
        char error[600];
        snprintf(error, sizeof(error), "unable to open %s", full_seq_path);
        perror(error);
        exit(1);
    }

    snprintf(full_key_path, sizeof(full_key_path), "%s/%s.key", dir, filename);
    fp2 = fopen(full_key_path, "rb");

    if (!fp2) {
        char error[600];
        fclose(fp2);
        snprintf(error, sizeof(error), "unable to open %s", full_key_path);
        perror(error);
        exit(1);
    }

    function(fp, fp2);

    fclose(fp);
    fclose(fp2);
}



void decode(const char *dir, const char *filename, void(*function)(FILE *))
{
    char full_path[512];
    FILE *fp;

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

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s path/to/gamedata\n", argv[0]);
        return 1;
    }

    // already converted:
    decode(argv[1], "crew.dat", write_historical_men);
    //decode(argv[1], "user.dat", write_custom_men);


    decode(argv[1], "rast.dat", write_player_data); // overlay for historical equipment


    decode(argv[1], "endgame.dat", write_endgame);
    decode(argv[1], "event.dat", write_events);
    decode(argv[1], "fails.cdr", write_failure_modes);
    decode(argv[1], "hist.dat", write_historical_equip); // overlay for historical equipment
    decode(argv[1], "help.cdr", write_help);
    // missSteps.dat -- already a text file
    decode(argv[1], "mission.dat", write_mission);
    decode(argv[1], "news.dat", write_news);
    decode(argv[1], "ntable.dat", write_vab_drawing_offsets);
    decode(argv[1], "p_rev.dat", write_post_mission_review);
    // rast.dat   -- RLED Players

    //decode(argv[1], "records.dat", write_records);      // This isn't really a data file
    decode(argv[1], "vtable.dat", write_vab_drawing_offsets);

    decode_seq(argv[1], "seq", write_mission_animation_success_sequence);
    decode_seq(argv[1], "fseq", write_mission_animation_failure_sequence);

    return 0;
}
