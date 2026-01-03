/******************************************************************************
 * Файл: photo_archive.c
 * Автор: Пичужкин Алексей (бТИИ-251)
 *
 * Описание: Консольное приложение для управления базой данных "Фотоархив".
 *           Позволяет хранить, просматривать, добавлять, искать и сортировать
 *           записи о фотографиях.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

 /* Константы для размеров массивов */
#define MAX_PHOTOS 100          /* Максимальное количество фотографий */
#define MAX_NAME_LEN 50         /* Максимальная длина названия */
#define MAX_PLACE_LEN 50        /* Максимальная длина места съемки */
#define MAX_CATEGORY_LEN 30     /* Максимальная длина категории */
#define MAX_TAGS_LEN 100        /* Максимальная длина тегов */
#define MAX_FORMAT_LEN 10       /* Максимальная длина формата файла */
#define FILENAME "photo_archive.txt"  /* Имя файла для сохранения данных */

/* Структура для хранения данных о фотографии */
typedef struct {
    char name[MAX_NAME_LEN];        /* Название фотографии */
    char date[11];                  /* Дата съемки в формате ГГГГ-ММ-ДД */
    char place[MAX_PLACE_LEN];      /* Место съемки */
    char category[MAX_CATEGORY_LEN];/* Категория */
    char tags[MAX_TAGS_LEN];        /* Теги (через запятую) */
    double size;                    /* Размер в МБ */
    int width;                      /* Ширина в пикселях */
    int height;                     /* Высота в пикселях */
    char format[MAX_FORMAT_LEN];    /* Формат файла (JPG, PNG и т.д.) */
} Photo;

/* Прототипы функций */
int initialize_program(void);
int load_database_from_file(Photo database[], int* record_count);
int save_database_to_file(const Photo database[], int record_count);
int display_all_records(const Photo database[], int record_count);
int add_photo_record(Photo database[], int* record_count);
int find_photos_by_location(const Photo database[], int record_count, const char* location);
int find_photos_by_date_and_tags(const Photo database[], int record_count,
    const char* date, const char* tag);
int sort_database_multi_level(Photo database[], int record_count);
int display_main_menu(int* user_selection);
int get_menu_selection(int* selection);
int clear_stdin_buffer(void);
int show_photo_information(const Photo* photo);
int compare_photos_for_sorting(const void* first_photo, const void* second_photo);
int prompt_for_enter_key(void);
int print_horizontal_separator(void);
int validate_date_format(const char* date);
int validate_positive_number(double number);
int validate_positive_integer(int number);

/******************************************************************************
 * Функция: main
 *
 * Описание: Главная функция программы. Организует основной цикл работы
 *           с меню и обработкой выбора пользователя.
 *
 * Возвращает: 0 при успешном завершении, 1 при ошибке инициализации
 ******************************************************************************/
int main(void)
{
    Photo photo_database[MAX_PHOTOS];   /* Массив для хранения фотографий */
    int photo_count = 0;                /* Текущее количество фотографий */
    int unsaved_changes = 0;            /* Флаг несохраненных изменений */
    int user_choice = 0;
    int program_exit = 0;
    int operation_result = 0;

    /* Инициализация программы */
    operation_result = initialize_program();
    if (operation_result != 0)
    {
        printf("Ошибка инициализации программы.\n");
        return 1;
    }

    /* Загрузка данных из файла */
    operation_result = load_database_from_file(photo_database, &photo_count);
    if (operation_result == -1)
    {
        printf("Внимание: Файл '%s' не найден. Создана новая база данных.\n", FILENAME);
    }
    else if (photo_count > 0)
    {
        printf("Данные успешно загружены из файла '%s'. Загружено %d записей.\n",
            FILENAME, photo_count);
    }
    else
    {
        printf("Файл '%s' существует, но не содержит корректных данных.\n", FILENAME);
    }

    prompt_for_enter_key();

    /* Основной цикл работы программы */
    while (program_exit == 0)
    {
        operation_result = display_main_menu(&user_choice);
        if (operation_result != 0)
        {
            printf("Ошибка отображения меню.\n");
            prompt_for_enter_key();
            continue;
        }

        switch (user_choice)
        {
        case 1:
            operation_result = display_all_records(photo_database, photo_count);
            if (operation_result != 0)
            {
                printf("Ошибка при отображении записей.\n");
            }
            prompt_for_enter_key();
            break;

        case 2:
            operation_result = add_photo_record(photo_database, &photo_count);
            if (operation_result == 0)
            {
                unsaved_changes = 1;
                printf("Фотография успешно добавлена в базу данных.\n");
            }
            else
            {
                printf("Не удалось добавить фотографию.\n");
            }
            prompt_for_enter_key();
            break;

        case 3:
        {
            char search_location[MAX_PLACE_LEN];
            printf("Введите место съемки для поиска: ");
            clear_stdin_buffer();
            fgets(search_location, MAX_PLACE_LEN, stdin);
            search_location[strcspn(search_location, "\n")] = '\0';

            operation_result = find_photos_by_location(photo_database, photo_count, search_location);
            if (operation_result < 0)
            {
                printf("Ошибка при поиске.\n");
            }
        }
        prompt_for_enter_key();
        break;

        case 4:
        {
            char search_date[11];
            char search_tag[50];

            printf("Введите дату для поиска (ГГГГ-ММ-ДД): ");
            clear_stdin_buffer();
            fgets(search_date, sizeof(search_date), stdin);
            search_date[strcspn(search_date, "\n")] = '\0';

            if (validate_date_format(search_date) != 0)
            {
                printf("Ошибка: Неверный формат даты.\n");
                prompt_for_enter_key();
                break;
            }

            printf("Введите тег для поиска: ");
            fgets(search_tag, sizeof(search_tag), stdin);
            search_tag[strcspn(search_tag, "\n")] = '\0';

            operation_result = find_photos_by_date_and_tags(photo_database, photo_count,
                search_date, search_tag);
            if (operation_result < 0)
            {
                printf("Ошибка при поиске.\n");
            }
        }
        prompt_for_enter_key();
        break;

        case 5:
            operation_result = sort_database_multi_level(photo_database, photo_count);
            if (operation_result == 0)
            {
                unsaved_changes = 1;
                printf("Сортировка выполнена успешно.\n");
            }
            else
            {
                printf("Не удалось выполнить сортировку.\n");
            }
            prompt_for_enter_key();
            break;

        case 6:
            operation_result = save_database_to_file(photo_database, photo_count);
            if (operation_result == 0)
            {
                unsaved_changes = 0;
                printf("Данные успешно сохранены в файл '%s'.\n", FILENAME);
            }
            else
            {
                printf("Ошибка: Не удалось сохранить данные в файл.\n");
            }
            prompt_for_enter_key();
            break;

        case 0:
            if (unsaved_changes != 0)
            {
                char save_confirmation;
                printf("\n==================================================\n");
                printf("ВНИМАНИЕ: Есть несохраненные изменения!\n");
                printf("Сохранить изменения перед выходом? (y/n): ");
                scanf(" %c", &save_confirmation);
                clear_stdin_buffer();

                if (save_confirmation == 'y' || save_confirmation == 'Y')
                {
                    operation_result = save_database_to_file(photo_database, photo_count);
                    if (operation_result == 0)
                    {
                        printf("Данные успешно сохранены.\n");
                    }
                    else
                    {
                        printf("Не удалось сохранить данные.\n");
                    }
                }
            }
            program_exit = 1;
            printf("\nДо свидания!\n");
            prompt_for_enter_key();
            break;

        default:
            printf("\nОшибка: Неверный выбор. Пожалуйста, введите число от 0 до 6.\n");
            prompt_for_enter_key();
            break;
        }
    }

    return 0;
}

/******************************************************************************
 * Функция: initialize_program
 *
 * Описание: Инициализирует программу, настраивает локализацию и выводит
 *           приветственное сообщение.
 *
 * Возвращает: 0 при успешной инициализации, -1 при ошибке
 ******************************************************************************/
int initialize_program(void)
{
    /* Настройка локализации для поддержки русского языка */
    setlocale(LC_ALL, "Russian");
    system("chcp 1251 > nul");

    print_horizontal_separator();
    printf("             БАЗА ДАННЫХ 'ФОТОАРХИВ'             \n");
    print_horizontal_separator();
    printf("Выполнил: Пичужкин Алексей (бТИИ-251)\n\n");
    print_horizontal_separator();

    return 0;
}

/******************************************************************************
 * Функция: load_database_from_file
 *
 * Описание: Загружает данные о фотографиях из текстового файла.
 *
 * Параметры:
 *   database - массив структур Photo для загрузки данных
 *   record_count - указатель на переменную для хранения количества записей
 *
 * Возвращает: 0 при успешной загрузке, -1 при ошибке открытия файла
 ******************************************************************************/
int load_database_from_file(Photo database[], int* record_count)
{
    FILE* file_handle = NULL;
    int records_loaded = 0;

    file_handle = fopen(FILENAME, "r");
    if (file_handle == NULL)
    {
        printf("Внимание: Не удалось загрузить данные из файла или файл не существует.\n");
        printf("Будет создана новая база данных.\n");
        *record_count = 0;
        return -1;
    }

    while (records_loaded < MAX_PHOTOS)
    {
        int result = fscanf(file_handle, "%49[^|]|%10[^|]|%49[^|]|%29[^|]|%99[^|]|%lf|%d|%d|%9s",
            database[records_loaded].name,
            database[records_loaded].date,
            database[records_loaded].place,
            database[records_loaded].category,
            database[records_loaded].tags,
            &database[records_loaded].size,
            &database[records_loaded].width,
            &database[records_loaded].height,
            database[records_loaded].format);

        if (result != 9)  /* Если не удалось прочитать все 9 полей */
            break;

        records_loaded++;

        /* Пропускаем оставшуюся часть строки (символ новой строки или пробелы) */
        int ch;
        while ((ch = fgetc(file_handle)) != '\n' && ch != EOF)
            ;
    }

    fclose(file_handle);
    *record_count = records_loaded;
    return 0;
}

/******************************************************************************
 * Функция: save_database_to_file
 *
 * Описание: Сохраняет данные о фотографиях в текстовый файл.
 *
 * Параметры:
 *   database - массив структур Photo для сохранения
 *   record_count - количество записей для сохранения
 *
 * Возвращает: 0 при успешном сохранении, -1 при ошибке открытия файла
 ******************************************************************************/
int save_database_to_file(const Photo database[], int record_count)
{
    FILE* file_handle = NULL;
    int i = 0;

    file_handle = fopen(FILENAME, "w");
    if (file_handle == NULL)
    {
        printf("Ошибка: Не удалось открыть файл '%s' для записи.\n", FILENAME);
        return -1;
    }

    for (i = 0; i < record_count; i++)
    {
        if (fprintf(file_handle, "%s|%s|%s|%s|%s|%.2f|%d|%d|%s\n",
            database[i].name,
            database[i].date,
            database[i].place,
            database[i].category,
            database[i].tags,
            database[i].size,
            database[i].width,
            database[i].height,
            database[i].format) < 0)
        {
            fclose(file_handle);
            printf("Ошибка записи в файл.\n");
            return -1;
        }
    }

    fclose(file_handle);
    return 0;
}

/******************************************************************************
 * Функция: display_all_records
 *
 * Описание: Выводит на экран все записи о фотографиях в табличном формате.
 *
 * Параметры:
 *   database - массив структур Photo для вывода
 *   record_count - количество записей для вывода
 *
 * Возвращает: 0 при успешном выводе, -1 если база данных пуста
 ******************************************************************************/
int display_all_records(const Photo database[], int record_count)
{
    int i = 0;

    if (record_count <= 0)
    {
        printf("База данных пуста.\n");
        return -1;
    }

    printf("\nВсего фотографий в базе: %d\n\n", record_count);
    print_horizontal_separator();
    printf("№  Название          Дата       Место          Категория   Размер   Разрешение Формат\n");
    print_horizontal_separator();

    for (i = 0; i < record_count; i++)
    {
        printf("%-3d%-17.17s%-12s%-15.15s%-12.12s%-8.2f  %dx%d   %s\n",
            i + 1,
            database[i].name,
            database[i].date,
            database[i].place,
            database[i].category,
            database[i].size,
            database[i].width,
            database[i].height,
            database[i].format);
    }

    print_horizontal_separator();
    return 0;
}

/******************************************************************************
 * Функция: add_photo_record
 *
 * Описание: Добавляет новую запись о фотографии в базу данных.
 *
 * Параметры:
 *   database - массив структур Photo
 *   record_count - указатель на переменную с текущим количеством записей
 *
 * Возвращает: 0 при успешном добавлении, -1 при ошибке
 ******************************************************************************/
int add_photo_record(Photo database[], int* record_count)
{
    Photo new_photo_record;
    int input_status = 0;
    char size_input[50];  /* Буфер для ввода размера как строки */

    if (*record_count >= MAX_PHOTOS)
    {
        printf("Ошибка: Достигнуто максимальное количество записей (%d).\n", MAX_PHOTOS);
        return -1;
    }

    printf("\nЗаполните информацию о новой фотографии:\n\n");
    clear_stdin_buffer();

    /* Ввод названия */
    printf("Введите название фотографии (до %d символов): ", MAX_NAME_LEN - 1);
    fgets(new_photo_record.name, MAX_NAME_LEN, stdin);
    new_photo_record.name[strcspn(new_photo_record.name, "\n")] = '\0';

    /* Ввод даты с проверкой */
    do {
        printf("Введите дату съемки (ГГГГ-ММ-ДД): ");
        fgets(new_photo_record.date, sizeof(new_photo_record.date), stdin);
        new_photo_record.date[strcspn(new_photo_record.date, "\n")] = '\0';

        if (validate_date_format(new_photo_record.date) != 0)
        {
            printf("Ошибка: Неверный формат даты. Используйте ГГГГ-ММ-ДД\n");
        }
    } while (validate_date_format(new_photo_record.date) != 0);

    /* Ввод места съемки */
    printf("Введите место съемки (до %d символов): ", MAX_PLACE_LEN - 1);
    fgets(new_photo_record.place, MAX_PLACE_LEN, stdin);
    new_photo_record.place[strcspn(new_photo_record.place, "\n")] = '\0';

    /* Ввод категории */
    printf("Введите категорию (до %d символов): ", MAX_CATEGORY_LEN - 1);
    fgets(new_photo_record.category, MAX_CATEGORY_LEN, stdin);
    new_photo_record.category[strcspn(new_photo_record.category, "\n")] = '\0';

    /* Ввод теги */
    printf("Введите теги через запятую (до %d символов): ", MAX_TAGS_LEN - 1);
    fgets(new_photo_record.tags, MAX_TAGS_LEN, stdin);
    new_photo_record.tags[strcspn(new_photo_record.tags, "\n")] = '\0';

    /* Ввод размера файла с проверкой - используем строку и заменяем запятую на точку */
    do {
        printf("Введите размер файла в МБ (можно использовать запятую или точку): ");
        fgets(size_input, sizeof(size_input), stdin);
        size_input[strcspn(size_input, "\n")] = '\0';

        /* Заменяем запятую на точку, если пользователь ввел запятую */
        for (int i = 0; size_input[i] != '\0'; i++) {
            if (size_input[i] == ',') {
                size_input[i] = '.';
            }
        }

        /* Пробуем преобразовать строку в число */
        char* endptr;
        new_photo_record.size = strtod(size_input, &endptr);

        if (endptr == size_input || *endptr != '\0') {
            printf("Ошибка: Неверный формат размера. Введите число (например: 4.50 или 4,50)\n");
            input_status = 0;
        }
        else if (validate_positive_number(new_photo_record.size) != 0) {
            printf("Ошибка: Размер должен быть положительным числом.\n");
            input_status = 0;
        }
        else {
            input_status = 1;
        }
    } while (input_status != 1);

    /* Ввод ширины с проверкой */
    do {
        printf("Введите ширину изображения в пикселях: ");
        input_status = scanf("%d", &new_photo_record.width);
        clear_stdin_buffer();

        if (input_status != 1)
        {
            printf("Ошибка: Неверный формат ширины. Введите целое число (например: 1920)\n");
        }
        else if (validate_positive_integer(new_photo_record.width) != 0)
        {
            printf("Ошибка: Ширина должна быть положительным числом.\n");
            input_status = 0;
        }
    } while (input_status != 1);

    /* Ввод высоты с проверкой */
    do {
        printf("Введите высоту изображения в пикселях: ");
        input_status = scanf("%d", &new_photo_record.height);
        clear_stdin_buffer();

        if (input_status != 1)
        {
            printf("Ошибка: Неверный формат высоты. Введите целое число (например: 1080)\n");
        }
        else if (validate_positive_integer(new_photo_record.height) != 0)
        {
            printf("Ошибка: Высота должна быть положительным числом.\n");
            input_status = 0;
        }
    } while (input_status != 1);

    /* Ввод формата файла */
    clear_stdin_buffer();
    printf("Введите формат файла (до %d символов): ", MAX_FORMAT_LEN - 1);
    fgets(new_photo_record.format, MAX_FORMAT_LEN, stdin);
    new_photo_record.format[strcspn(new_photo_record.format, "\n")] = '\0';

    /* Добавление новой фотографии в массив */
    database[*record_count] = new_photo_record;
    (*record_count)++;

    return 0;
}

/******************************************************************************
 * Функция: find_photos_by_location
 *
 * Описание: Выполняет поиск фотографий по месту съемки.
 *
 * Параметры:
 *   database - массив структур Photo для поиска
 *   record_count - количество записей в массиве
 *   location - строка с местом для поиска
 *
 * Возвращает: количество найденных фотографий, -1 если база данных пуста
 ******************************************************************************/
int find_photos_by_location(const Photo database[], int record_count, const char* location)
{
    int i = 0;
    int found_records = 0;

    if (record_count <= 0)
    {
        printf("База данных пуста.\n");
        return -1;
    }

    if (location == NULL || strlen(location) == 0)
    {
        printf("Ошибка: Не задано место для поиска.\n");
        return -1;
    }

    printf("\nРезультаты поиска для места: '%s'\n", location);
    print_horizontal_separator();

    for (i = 0; i < record_count; i++)
    {
        /* Поиск подстроки (регистрозависимый) */
        if (strstr(database[i].place, location) != NULL)
        {
            printf("%d. %s (Дата: %s, Категория: %s)\n",
                found_records + 1,
                database[i].name,
                database[i].date,
                database[i].category);
            found_records++;
        }
    }

    print_horizontal_separator();

    if (found_records == 0)
    {
        printf("Фотографии с указанным местом съемки не найдены.\n");
    }
    else
    {
        printf("\nНайдено фотографий: %d\n", found_records);
    }

    return found_records;
}

/******************************************************************************
 * Функция: find_photos_by_date_and_tags
 *
 * Описание: Выполняет комбинированный поиск по дате и тегам.
 *
 * Параметры:
 *   database - массив структур Photo для поиска
 *   record_count - количество записей в массиве
 *   date - дата для поиска (формат ГГГГ-ММ-ДД)
 *   tag - тег для поиска
 *
 * Возвращает: количество найденных фотографий, -1 при ошибке
 ******************************************************************************/
int find_photos_by_date_and_tags(const Photo database[], int record_count,
    const char* date, const char* tag)
{
    int i = 0;
    int found_records = 0;

    if (record_count <= 0)
    {
        printf("База данных пуста.\n");
        return -1;
    }

    if (date == NULL || tag == NULL)
    {
        printf("Ошибка: Не заданы параметры поиска.\n");
        return -1;
    }

    if (validate_date_format(date) != 0)
    {
        printf("Ошибка: Неверный формат даты.\n");
        return -1;
    }

    printf("\nРезультаты поиска для даты '%s' и тега '%s':\n", date, tag);
    print_horizontal_separator();

    for (i = 0; i < record_count; i++)
    {
        if (strcmp(database[i].date, date) == 0 &&
            strstr(database[i].tags, tag) != NULL)
        {
            printf("%d. %s (Место: %s, Категория: %s)\n",
                found_records + 1,
                database[i].name,
                database[i].place,
                database[i].category);
            printf("   Теги: %s\n", database[i].tags);
            printf("   Разрешение: %dx%d, Размер: %.2f МБ\n",
                database[i].width,
                database[i].height,
                database[i].size);
            print_horizontal_separator();
            found_records++;
        }
    }

    if (found_records == 0)
    {
        printf("Фотографии с указанной датой и тегом не найдены.\n");
    }
    else
    {
        printf("\nНайдено фотографий: %d\n", found_records);
    }

    return found_records;
}

/******************************************************************************
 * Функция: sort_database_multi_level
 *
 * Описание: Выполняет многоуровневую сортировку фотографий по дате,
 *           категории и разрешению (ширина × высота).
 *
 * Параметры:
 *   database - массив структур Photo для сортировки
 *   record_count - количество записей в массиве
 *
 * Возвращает: 0 при успешной сортировке, -1 при ошибке
 ******************************************************************************/
int sort_database_multi_level(Photo database[], int record_count)
{
    if (record_count <= 1)
    {
        printf("Нечего сортировать. В базе данных %d записей.\n", record_count);
        return -1;
    }

    qsort(database, record_count, sizeof(Photo), compare_photos_for_sorting);
    return 0;
}

/******************************************************************************
 * Функция: compare_photos_for_sorting
 *
 * Описание: Функция сравнения для qsort. Сравнивает фотографии по дате,
 *           затем по категории, затем по разрешению (произведение ширины на высоту).
 *
 * Параметры:
 *   first_photo - указатель на первую фотографию
 *   second_photo - указатель на вторую фотографию
 *
 * Возвращает: отрицательное число если first < second,
 *             0 если first == second,
 *             положительное если first > second
 ******************************************************************************/
int compare_photos_for_sorting(const void* first_photo, const void* second_photo)
{
    const Photo* photo_a = (const Photo*)first_photo;
    const Photo* photo_b = (const Photo*)second_photo;
    int date_comparison = 0;
    int category_comparison = 0;
    int resolution_a = 0;
    int resolution_b = 0;

    /* Сравнение по дате */
    date_comparison = strcmp(photo_a->date, photo_b->date);
    if (date_comparison != 0)
    {
        return date_comparison;
    }

    /* Если даты равны, сравнение по категории */
    category_comparison = strcmp(photo_a->category, photo_b->category);
    if (category_comparison != 0)
    {
        return category_comparison;
    }

    /* Если категории равны, сравнение по разрешению */
    resolution_a = photo_a->width * photo_a->height;
    resolution_b = photo_b->width * photo_b->height;

    return resolution_a - resolution_b;
}

/******************************************************************************
 * Функция: display_main_menu
 *
 * Описание: Выводит на экран главное меню программы и получает выбор пользователя.
 *
 * Параметры:
 *   user_selection - указатель на переменную для хранения выбора пользователя
 *
 * Возвращает: 0 при успешном отображении, -1 при ошибке
 ******************************************************************************/
int display_main_menu(int* user_selection)
{
    int menu_selection = 0;

    if (user_selection == NULL)
    {
        return -1;
    }

    system("cls");
    printf("\n");
    print_horizontal_separator();
    printf("          ГЛАВНОЕ МЕНЮ ФОТОАРХИВА          \n");
    print_horizontal_separator();
    printf("1. Просмотр всех фотографий\n");
    printf("2. Добавить новую фотографию\n");
    printf("3. Поиск по месту съемки\n");
    printf("4. Комбинированный поиск (дата + теги)\n");
    printf("5. Многоуровневая сортировка\n");
    printf("6. Сохранить изменения в файл\n");
    printf("0. Выход из программы\n");
    print_horizontal_separator();
    printf("\nВыберите действие (0-6): ");

    if (get_menu_selection(&menu_selection) != 0)
    {
        return -1;
    }

    *user_selection = menu_selection;
    return 0;
}

/******************************************************************************
 * Функция: get_menu_selection
 *
 * Описание: Получает выбор пользователя из меню.
 *
 * Параметры:
 *   selection - указатель на переменную для хранения выбора
 *
 * Возвращает: 0 при успешном получении, -1 при ошибке
 ******************************************************************************/
int get_menu_selection(int* selection)
{
    int input_result = 0;

    if (selection == NULL)
    {
        return -1;
    }

    input_result = scanf("%d", selection);
    clear_stdin_buffer();

    if (input_result != 1)
    {
        *selection = -1;
        return -1;
    }

    return 0;
}

/******************************************************************************
 * Функция: clear_stdin_buffer
 *
 * Описание: Очищает буфер ввода стандартного потока.
 *
 * Возвращает: количество очищенных символов
 ******************************************************************************/
int clear_stdin_buffer(void)
{
    int character = 0;
    int cleared_count = 0;

    while ((character = getchar()) != '\n' && character != EOF)
    {
        cleared_count++;
    }

    return cleared_count;
}

/******************************************************************************
 * Функция: show_photo_information
 *
 * Описание: Выводит подробную информацию об одной фотографии.
 *
 * Параметры:
 *   photo - указатель на структуру Photo для вывода
 *
 * Возвращает: 0 при успешном выводе, -1 при ошибке
 ******************************************************************************/
int show_photo_information(const Photo* photo)
{
    if (photo == NULL)
    {
        return -1;
    }

    print_horizontal_separator();
    printf("     ПОДРОБНАЯ ИНФОРМАЦИЯ О ФОТОГРАФИИ     \n");
    print_horizontal_separator();
    printf("Название: %s\n", photo->name);
    printf("Дата съемки: %s\n", photo->date);
    printf("Место съемки: %s\n", photo->place);
    printf("Категория: %s\n", photo->category);
    printf("Теги: %s\n", photo->tags);
    printf("Размер файла: %.2f МБ\n", photo->size);
    printf("Разрешение: %d x %d пикселей\n", photo->width, photo->height);
    printf("Формат файла: %s\n", photo->format);
    print_horizontal_separator();

    return 0;
}

/******************************************************************************
 * Функция: prompt_for_enter_key
 *
 * Описание: Ожидает нажатия клавиши Enter для продолжения.
 *
 * Возвращает: 0 при успешном ожидании
 ******************************************************************************/
int prompt_for_enter_key(void)
{
    printf("\nДля продолжения нажмите Enter...");
    clear_stdin_buffer();
    getchar();
    return 0;
}

/******************************************************************************
 * Функция: print_horizontal_separator
 *
 * Описание: Выводит разделительную линию.
 *
 * Возвращает: количество выведенных символов
 ******************************************************************************/
int print_horizontal_separator(void)
{
    int i = 0;

    for (i = 0; i < 50; i++)
    {
        printf("=");
    }
    printf("\n");

    return i;
}

/******************************************************************************
 * Функция: validate_date_format
 *
 * Описание: Проверяет корректность формата даты ГГГГ-ММ-ДД.
 *
 * Параметры:
 *   date - строка с датой для проверки
 *
 * Возвращает: 0 если формат корректен, -1 если некорректен
 ******************************************************************************/
int validate_date_format(const char* date)
{
    int year = 0;
    int month = 0;
    int day = 0;

    if (date == NULL || strlen(date) != 10)
    {
        return -1;
    }

    if (date[4] != '-' || date[7] != '-')
    {
        return -1;
    }

    if (sscanf(date, "%4d-%2d-%2d", &year, &month, &day) != 3)
    {
        return -1;
    }

    if (year < 1900 || year > 2100)
    {
        return -1;
    }

    if (month < 1 || month > 12)
    {
        return -1;
    }

    if (day < 1 || day > 31)
    {
        return -1;
    }

    return 0;
}

/******************************************************************************
 * Функция: validate_positive_number
 *
 * Описание: Проверяет, является ли число положительным.
 *
 * Параметры:
 *   number - число для проверки
 *
 * Возвращает: 0 если число положительное, -1 если нет
 ******************************************************************************/
int validate_positive_number(double number)
{
    if (number > 0)
    {
        return 0;
    }

    return -1;
}

/******************************************************************************
 * Функция: validate_positive_integer
 *
 * Описание: Проверяет, является ли целое число положительным.
 *
 * Параметры:
 *   number - целое число для проверки
 *
 * Возвращает: 0 если число положительное, -1 если нет
 ******************************************************************************/
int validate_positive_integer(int number)
{
    if (number > 0)
    {
        return 0;
    }

    return -1;
}