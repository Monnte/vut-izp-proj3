#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

int check_maze_enter(Map *map, int r, int c); //Checking arguments for enter to maze , must enter from border
int map_init(Map *map, int rows, int cols);
int map_free(Map *map);
int load_map(Map *mapa, FILE *map);
int generate_map(Map *mapa, char *filename);                                                         //calling mapinit + load map
int map_isvalid(char *filenmae);                                                                     // Check if map is valid
bool is_cell_border(char c, int border);                                                             //Checking border by char
bool is_border(Map *map, int r, int c, int border);                                                  //Checking border by row and column coordinates
int start_border(Map *map, int r, int c, int leftright);                                             //Returning first border to catch
void get_move_vector(Map *map, int r, int c, int *border, int *direction, int leftright, int *move); //Get next move in maze
int findpath(Map *map, int r, int c, int leftright);                                                 //Finding path from maze

int main(int argc, char *argv[])
{
    Map Mapa;

    if (argc == 2)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            char *helptext =
                "____________________________________________|HELP|____________________________________________\n\n\n"
                "--test <filepath>: Check if map is valid\n\n"
                "--rpath <row> <column> <filepath>: Finding way out from maze with right hand rule\n\n"
                "--lpath <row> <column> <filepath>: Finding way out from maze with left hand rule\n\n"
                "--shortest <row> <column> <filepath>: Finding shortest way out from maze\n";
            printf("%s", helptext);

            return 0;
        }
    }

    if (argc == 3)
    {

        if (strcmp(argv[1], "--test") == 0)
        {
            if (!map_isvalid(argv[2]))
                printf("Valid\n");
            else
                printf("Invalid\n");

            return 0;
        }
    }

    if (argc == 5)
    {

        if (strcmp(argv[1], "--rpath") == 0)
        {
            if (!atoi(argv[2]) || !atoi(argv[3]))
            {
                fprintf(stderr, "Not appropriate arguments.\n");
                return 1;
            }

            if (map_isvalid(argv[4]))
            {
                fprintf(stderr, "Something went wrong. Map is not Valid\n");
                return 1;
            }

            if (!generate_map(&Mapa, argv[4]))
            {
                if (check_maze_enter(&Mapa, atoi(argv[2]), atoi(argv[3])))
                {
                    fprintf(stderr, "You need to enter to map from border\n");
                    map_free(&Mapa);
                    return 1;
                }
                if (findpath(&Mapa, atoi(argv[2]), atoi(argv[3]), -1) == -1)
                {
                    fprintf(stderr, "You can't enter to maze because there is a border.\n");
                    map_free(&Mapa);
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Something went wrong try again.\n");
                map_free(&Mapa);
                return 1;
            }

            map_free(&Mapa);
            return 0;
        }
        else if (strcmp(argv[1], "--lpath") == 0)
        {

            if (!atoi(argv[2]) || !atoi(argv[3]))
            {
                fprintf(stderr, "Not appropriate arguments.\n");
                return 1;
            }

            if (map_isvalid(argv[4]))
            {
                fprintf(stderr, "Something went wrong. Map is not Valid\n");
                return 1;
            }

            if (!generate_map(&Mapa, argv[4]))
            {
                if (check_maze_enter(&Mapa, atoi(argv[2]), atoi(argv[3])))
                {
                    fprintf(stderr, "You need to enter to map from border\n");
                    map_free(&Mapa);
                    return 1;
                }
                if (findpath(&Mapa, atoi(argv[2]), atoi(argv[3]), 1) == -1)
                {
                    fprintf(stderr, "You can't enter to maze because there is a border.\n");
                    map_free(&Mapa);
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Something went wrong try again.\n");
                map_free(&Mapa);
                return 1;
            }

            map_free(&Mapa);
            return 0;
        }
        else if (strcmp(argv[1], "--shortest") == 0)
        {
            fprintf(stderr, "I have a theory that when you're lost on the path, go back to the beginning and try the maze again.\n");
            return 1;
        }
    }

    fprintf(stderr, "Something went wrong. Try --help\n");
    return 1;
}

int findpath(Map *map, int r, int c, int leftright)
{
    int move[2];
    int border = start_border(map, r, c, leftright);
    int direction = (r % 2 == c % 2) ? 1 : -1;

    if (border == -1)
        return -1;

    //While inside in maze find new move
    while (r > 0 && r <= map->rows && c > 0 && c <= map->cols)
    {
        printf("%d,%d\n", r, c);

        get_move_vector(map, r, c, &border, &direction, leftright, move);
        r += move[1];
        c += move[0];
    }

    return 0;
}

void get_move_vector(Map *map, int r, int c, int *border, int *direction, int leftright, int *move)
{
    int crossed = -1;

    for (int i = 0; i < 3 && crossed < 0; i++)
    {
        if (!is_border(map, r, c, (*border + (i * *direction * leftright) + 3) % 3))
        {
            crossed = (*border + (i * *direction * leftright) + 3) % 3;
        }
    }

    switch (crossed)
    {
    case 0:
        *border = 1;
        move[0] = -1;
        move[1] = 0;
        break;
    case 1:
        *border = 0;
        move[0] = 1;
        move[1] = 0;
        break;
    case 2:
        if (r % 2 == c % 2)
        {
            move[0] = 0;
            move[1] = -1;
        }
        else
        {
            move[0] = 0;
            move[1] = 1;
        }
        *border = 2;
        break;
    default:
        break;
    }

    *direction = (r % 2 == c % 2) ? 1 : -1;
    *border = (*border + *direction * leftright + 3) % 3;
}

int start_border(Map *map, int r, int c, int leftright) // 1 right // -1 left
{
    int even = r % 2;
    int cols_even = map->cols % 2;
    int border;
    bool corner = false;

    if ((r == 1 && c == 1) || (r == 1 && c == map->cols) || (r == map->rows && c == 1) || (r == map->rows && c == map->cols))
    {
        if (!is_border(map, r, c, 2))
            corner = true;
    }

    if ((r == 1 && c != 1 && c != map->cols) || (r == 1 && corner)) //Enterence from TOP
    {
        border = leftright == -1 ? 1 : 0;
        if (is_border(map, r, c, 2))
            return -1;
    }
    else if ((r == map->rows && c != 1 && c != map->cols) || (r == map->rows && corner)) //Enterence from BOTTOM
    {
        border = leftright == -1 ? 0 : 1;
        if (is_border(map, r, c, 2))
            return -1;
    }
    else if (c == 1 && even == 1) //Enterence from LEFT on ODD line
    {
        border = leftright == -1 ? 2 : 1;
        if (is_border(map, r, c, 0))
            return -1;
    }
    else if (c == 1 && even == 0) //Enterence from LEFT on EVEN line
    {
        border = leftright == -1 ? 1 : 2;
        if (is_border(map, r, c, 0))
            return -1;
    }
    else if (c == map->cols && cols_even == 1) //Enterence from RIGHT on ODD line
    {
        border = leftright == -1 ? 0 : 2;
        if (is_border(map, r, c, 1))
            return -1;
    }
    else if (c == map->cols && cols_even == 0) //Enterence from RIGHT on EVEN line
    {
        border = leftright == -1 ? 2 : 0;
        if (is_border(map, r, c, 1))
            return -1;
    }

    if ((r == 1 && c != 1 && c != map->cols && c % 2 == 0) || (r == map->rows && c != 1 && c != map->cols && c % 2 == 0))
    {
        return -1;
    }

    //printf("StartBorder: %d\n",border);
    return border;
}

bool is_border(Map *map, int r, int c, int border)
{
    char cell = map->cells[map->cols * (r - 1) + c - 1];

    return is_cell_border(cell, border);
}

bool is_cell_border(char c, int border)
{
    int border_bool[3] = {0, 0, 0};
    c = c - '0';
    for (int i = 0; c != 0 && i != 3; i++)
    {
        border_bool[i] = c % 2;
        c = c / 2;
    }

    return border_bool[border];
}

int map_isvalid(char *filename)
{
    FILE *f_map;
    f_map = fopen(filename, "r");
    if (f_map == NULL)
        return 1; //File is not valid

    int rows, cols;

    if (fscanf(f_map, "%d", &rows) != 1)
    {
        fclose(f_map);
        return 1;
    }

    if (fscanf(f_map, "%d", &cols) != 1)
    {
        fclose(f_map);
        return 1;
    }

    if (cols <= 0 && rows <= 0)
    {
        fclose(f_map);
        return 1;
    }

    char *map;
    char *line;
    map = malloc((rows * cols + 1) * sizeof(char));
    line = malloc(sizeof(char) * (cols * 2 + 2));
    int i = 0;

    //first line check
    char c;
    while (c != '\n')
    {
        c = fgetc(f_map);
        if (c != ' ' && c != '\n')
        {
            free(map);
            free(line);
            fclose(f_map);
            return 1;
        }
    }

    int row_count = 0;
    while (fgets(line, cols * 2 + 2, f_map) != NULL)
    {
        if (row_count >= rows)
        {
            free(map);
            free(line);
            fclose(f_map);

            return 1;
        }

        line[strcspn(line, "\n")] = '\0';

        int char_count_in_line = 0;
        for (int j = 0; line[j] != '\0'; j++)
        {
            if (line[j] == ' ')
                continue;

            if (char_count_in_line >= cols)
            {
                free(map);
                free(line);
                fclose(f_map);

                return 1;
            }

            if (line[j] > '7' || line[j] < 0)
            {
                free(map);
                free(line);
                fclose(f_map);
                return 1;
            }
            else
            {
                map[i] = line[j];
                char_count_in_line++;
                i++;
            }
        }
        row_count++;
    }
    free(line);

    int rows_even = rows % 2;
    for (int j = 0; j != rows * cols; j++)
    {
        if ((j + 1) % cols != 0)
        {
            if (is_cell_border(map[j], 1) != is_cell_border(map[j + 1], 0))
            {
                free(map);
                fclose(f_map);
                return 1;
            }
        }

        if (((j - 1) / cols) % 2 == 1 && ((j - 1) / cols) != rows - 1)
        {
            if (j % 2 != rows_even)
            {
                if (is_cell_border(map[j], 2) != is_cell_border(map[j + cols], 2))
                {
                    free(map);
                    fclose(f_map);
                    return 1;
                }
            }
            else
            {
                if (is_cell_border(map[j], 2) != is_cell_border(map[j - cols], 2))
                {
                    free(map);
                    fclose(f_map);
                    return 1;
                }
            }
        }
    }

    free(map);
    if (i != rows * cols)
    {
        fclose(f_map);
        return 1;
    }

    fclose(f_map);
    return 0;
}

int generate_map(Map *mapa, char *filename)
{
    FILE *f_map;
    f_map = fopen(filename, "r");
    if (f_map == NULL)
        return 1;

    int rows, cols;
    fscanf(f_map, "%d", &rows);
    fscanf(f_map, "%d", &cols);

    map_init(mapa, rows, cols);
    load_map(mapa, f_map);

    fclose(f_map);
    return 0;
}

int load_map(Map *mapa, FILE *map)
{
    char c;
    int i = 0;
    while ((c = fgetc(map)) != EOF)
    {
        if (c != ' ' && c != '\n')
            mapa->cells[i++] = c;
    }
    mapa->cells[i] = '\0';

    return 0;
}

int map_init(Map *map, int rows, int cols)
{
    map->rows = rows;
    map->cols = cols;
    map->cells = malloc((rows * cols + 1) * sizeof(char));

    return 0;
}

int map_free(Map *map)
{
    map->rows = 0;
    map->cols = 0;
    free(map->cells);

    return 0;
}

int check_maze_enter(Map *map, int r, int c)
{
    if (r != 1 && r != map->rows && c != 1 && c != map->cols)
    {
        return 1;
    }
    if (r > map->rows || c > map->cols)
    {
        return 1;
    }

    return 0;
}