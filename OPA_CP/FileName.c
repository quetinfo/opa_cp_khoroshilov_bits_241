/*
 * Программа расчета значений функций 
 * Автор:  Хорошилов Павел Алексеевич 
 */

/* Подключение заголовочных файлов */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include <time.h>
#include <stdbool.h>
#include <float.h>

/* Объявление константы */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

 /* Параметры расчета */
#define DEFAULT_XMIN 0.1
#define DEFAULT_XMAX 5.0
#define DEFAULT_DX 0.5
#define DEFAULT_N 10

/* Режимы работы программы */
typedef enum {
    MODE_FILE = 1,
    MODE_STEP,
    MODE_RANDOM
} calculation_mode_t;

/* Глобальные переменные */
static double xmin = DEFAULT_XMIN;
static double xmax = DEFAULT_XMAX;
static double dx = DEFAULT_DX;
static int n = DEFAULT_N;
static calculation_mode_t current_mode = MODE_STEP;
static bool results_exist = false;
static double* x_values = NULL;
static double* f1_values = NULL;
static double* f2_values = NULL;
static double* f3_values = NULL;
static int values_count = 0;

/* Прототипы функций */
static double calculate_f1(double x);
static double calculate_f2(double x);
static double calculate_f3(double x);
static void calculate_functions(void);
static void free_memory(void);
static void print_table(bool to_file);
static void find_min_max(void);
static void linear_search(void);
static void differentiate_function(void);
static void set_calculation_parameters(void);
static void save_results_to_file(const char* filename);
static void show_menu(void);

/*
 * Вычисляет значение функции f1(x)
 * x - аргумент функции
 * Возвращает: вычисленное значение или NAN при ошибке
 */
static double calculate_f1(double x) {
    if (x <= 0 || x == 1) return NAN;
    double term = x / (M_PI * log(x));
    return -fabs(term) * cos(x);
}

/*
 * Вычисляет значение функции f2(x)
 * x - аргумент функции
 * Возвращает: вычисленное значение или NAN при ошибке
 */
static double calculate_f2(double x) {
    if (x < -1) {
        return exp(-3 * x);
    }
    else if (x >= -1 && x <= 1) {
        double term = 2 * x - 3 * atan(fabs(x));
        return (term >= 0) ? pow(term, 2.0 / 3.0) : NAN;
    }
    else if (x > 3) {
        return pow(1 + fabs(cos(x)), 1.0 / 3.0);
    }
    else {
        return NAN;
    }
}

/*
 * Вычисляет значение функции f3(x)
 * x - аргумент функции
 * Возвращает: вычисленное значение или NAN при ошибке
 */
static double calculate_f3(double x) {
    return (fabs(sin(x)) > 1e-10) ? 1.0 / sin(x) : NAN;
}

/*
 * Вычисляет значения функций для всех x в заданном диапазоне
 * согласно текущему режиму работы
 */
static void calculate_functions(void) {
    free_memory();

    if (current_mode == MODE_FILE) {
        FILE* input = fopen("input.csv", "r");
        if (!input) {
            printf("Ошибка открытия файла input.csv!\n");
            return;
        }

        /* Считает количество строк */
        values_count = 0;
        char buffer[100];
        while (fgets(buffer, sizeof(buffer), input)) values_count++;
        rewind(input);

        /* Выделяет память */
        x_values = malloc(values_count * sizeof(double));
        f1_values = malloc(values_count * sizeof(double));
        f2_values = malloc(values_count * sizeof(double));
        f3_values = malloc(values_count * sizeof(double));

        /* Читает и вычисляет */
        for (int i = 0; i < values_count; i++) {
            if (fscanf(input, "%lf", &x_values[i]) != 1) {
                printf("Ошибка чтения файла!\n");
                free_memory();
                fclose(input);
                return;
            }
            f1_values[i] = calculate_f1(x_values[i]);
            f2_values[i] = calculate_f2(x_values[i]);
            f3_values[i] = calculate_f3(x_values[i]);
        }
        fclose(input);
    }
    else if (current_mode == MODE_STEP) {
        values_count = n;
        x_values = malloc(values_count * sizeof(double));
        f1_values = malloc(values_count * sizeof(double));
        f2_values = malloc(values_count * sizeof(double));
        f3_values = malloc(values_count * sizeof(double));

        double x = xmin;
        for (int i = 0; i < values_count; i++) {
            x_values[i] = x;
            f1_values[i] = calculate_f1(x);
            f2_values[i] = calculate_f2(x);
            f3_values[i] = calculate_f3(x);
            x += dx;
        }
    }
    else if (current_mode == MODE_RANDOM) {
        values_count = n;
        x_values = malloc(values_count * sizeof(double));
        f1_values = malloc(values_count * sizeof(double));
        f2_values = malloc(values_count * sizeof(double));
        f3_values = malloc(values_count * sizeof(double));

        srand(time(NULL));
        for (int i = 0; i < values_count; i++) {
            x_values[i] = xmin + (xmax - xmin) * rand() / (double)RAND_MAX;
            f1_values[i] = calculate_f1(x_values[i]);
            f2_values[i] = calculate_f2(x_values[i]);
            f3_values[i] = calculate_f3(x_values[i]);
        }
    }

    results_exist = true;
    printf("Расчет завершен. Получено %d значений.\n", values_count);
}

/*
 * Освобождает память, выделенную под результаты вычислений
 */
static void free_memory(void) {
    if (x_values) free(x_values);
    if (f1_values) free(f1_values);
    if (f2_values) free(f2_values);
    if (f3_values) free(f3_values);
    x_values = NULL;
    f1_values = NULL;
    f2_values = NULL;
    f3_values = NULL;
    values_count = 0;
    results_exist = false;
}

/*
 * Выводит таблицу значений на экран или в файл
 * to_file - если true, сохраняет в файл, иначе выводит на экран
 */
static void print_table(bool to_file) {
    if (!results_exist) {
        printf("Нет данных для вывода. Сначала выполните расчет.\n");
        return;
    }

    if (to_file) {
        save_results_to_file("output_table.csv");
        printf("Таблица сохранена в output_table.csv\n");
    }
    else {
        printf("\n%10s %15s %15s %15s\n", "x", "f1(x)", "f2(x)", "f3(x)");
        for (int i = 0; i < values_count; i++) {
            printf("%10.6f %15.6f %15.6f %15.6f\n",
                x_values[i], f1_values[i], f2_values[i], f3_values[i]);
        }
    }
}

/*
 * Находит минимальные и максимальные значения функций
 */
static void find_min_max(void) {
    if (!results_exist) {
        printf("Нет данных для анализа. Сначала выполните расчет.\n");
        return;
    }

    double min_f1 = DBL_MAX;
    double max_f1 = -DBL_MAX;
    double min_f2 = DBL_MAX;
    double max_f2 = -DBL_MAX;
    double min_f3 = DBL_MAX;
    double max_f3 = -DBL_MAX;

    for (int i = 0; i < values_count; i++) {
        if (!isnan(f1_values[i])) {
            if (f1_values[i] < min_f1) min_f1 = f1_values[i];
            if (f1_values[i] > max_f1) max_f1 = f1_values[i];
        }
        if (!isnan(f2_values[i])) {
            if (f2_values[i] < min_f2) min_f2 = f2_values[i];
            if (f2_values[i] > max_f2) max_f2 = f2_values[i];
        }
        if (!isnan(f3_values[i])) {
            if (f3_values[i] < min_f3) min_f3 = f3_values[i];
            if (f3_values[i] > max_f3) max_f3 = f3_values[i];
        }
    }

    printf("\nМинимальные и максимальные значения:\n");
    printf("f1: min = %.6f, max = %.6f\n", min_f1, max_f1);
    printf("f2: min = %.6f, max = %.6f\n", min_f2, max_f2);
    printf("f3: min = %.6f, max = %.6f\n", min_f3, max_f3);
}

/*
 * Выполняет линейный поиск аргумента по значению функции
 */
static void linear_search(void) {
    if (!results_exist) {
        printf("Нет данных для поиска. Сначала выполните расчет.\n");
        return;
    }

    int func_choice;
    double target;
    double epsilon;

    printf("\nВыберите функцию (1-f1, 2-f2, 3-f3): ");
    scanf("%d", &func_choice);
    printf("Введите целевое значение функции: ");
    scanf("%lf", &target);
    printf("Введите точность: ");
    scanf("%lf", &epsilon);

    bool found = false;
    for (int i = 0; i < values_count; i++) {
        double current = 0;
        switch (func_choice) {
        case 1: current = f1_values[i]; break;
        case 2: current = f2_values[i]; break;
        case 3: current = f3_values[i]; break;
        }

        if (!isnan(current) && fabs(current - target) < epsilon) {
            printf("Найдено: x = %.6f, f%d(x) = %.6f\n",
                x_values[i], func_choice, current);
            found = true;
        }
    }

    if (!found) {
        printf("Значение не найдено с заданной точностью.\n");
    }
}

/*
 * Вычисляет производную функции в точке методом конечных разностей
 */
static void differentiate_function(void) {
    int func_choice;
    double x;
    double h;

    printf("\nВыберите функцию (1-f1, 2-f2, 3-f3): ");
    scanf("%d", &func_choice);
    printf("Введите точку дифференцирования: ");
    scanf("%lf", &x);
    printf("Введите шаг h: ");
    scanf("%lf", &h);

    double (*func)(double) = NULL;
    switch (func_choice) {
    case 1: func = calculate_f1; break;
    case 2: func = calculate_f2; break;
    case 3: func = calculate_f3; break;
    }

    if (!func) {
        printf("Неверный выбор функции!\n");
        return;
    }

    double f_x = func(x);
    double f_x_plus_h = func(x + h);

    if (isnan(f_x) || isnan(f_x_plus_h)) {
        printf("Функция не определена в указанных точках!\n");
        return;
    }

    double derivative = (f_x_plus_h - f_x) / h;
    printf("Производная f%d в точке %.6f: %.6f\n", func_choice, x, derivative);
}

/*
 * Устанавливает параметры расчета
 */
static void set_calculation_parameters(void) {
    printf("\nТекущие параметры расчета:\n");
    printf("Режим: %d (1-файл, 2-шаг, 3-случайные)\n", current_mode);
    printf("xmin: %.6f\n", xmin);
    printf("xmax: %.6f\n", xmax);
    printf("Шаг dx: %.6f\n", dx);
    printf("Количество значений n: %d\n", n);

    printf("\nВыберите режим (1-файл, 2-шаг, 3-случайные): ");
    scanf("%d", &current_mode);

    if (current_mode == MODE_STEP || current_mode == MODE_RANDOM) {
        printf("Введите xmin: ");
        scanf("%lf", &xmin);
        printf("Введите xmax: ");
        scanf("%lf", &xmax);

        if (current_mode == MODE_STEP) {
            printf("Введите шаг dx: ");
            scanf("%lf", &dx);
        }
        else {
            printf("Введите количество значений n: ");
            scanf("%d", &n);
        }
    }

    printf("Параметры успешно обновлены.\n");
}

/*
 * Сохраняет результаты вычислений в файл
 * filename - имя файла для сохранения
 */
static void save_results_to_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Ошибка создания файла!\n");
        return;
    }

    fprintf(file, "x,f1(x),f2(x),f3(x)\n");
    for (int i = 0; i < values_count; i++) {
        fprintf(file, "%.6f,%.6f,%.6f,%.6f\n",
            x_values[i], f1_values[i], f2_values[i], f3_values[i]);
    }

    fclose(file);
}

/*
 * Отображает главное меню программы
 */
static void show_menu(void) {
    printf("Меню:\n");
    printf("1) Вывод таблицы значений на экран или в файл\n");
    printf("2) Определение минимального и максимального значений\n");
    printf("3) Поиск аргумента по значению функции\n");
    printf("4) Дифференцирование функции в точке\n");
    printf("5) Задание параметров расчета\n");
    printf("6) Индивидуальное задание\n");
    printf("7) Завершение работы\n");
    printf("Выберите пункт: ");
}

/*
 * Точка входа в программу
 */
int main(void) {
    setlocale(LC_ALL, "Russian");

    printf("Программа расчета значений функции\n");

    int choice;

    while (1) {
        show_menu();
        scanf("%d", &choice);

        switch (choice) {
        case 1: {
            int subchoice;
            printf("Вывести на экран (1) или в файл (2)? ");
            scanf("%d", &subchoice);
            print_table(subchoice == 2);
            break;
        }
        case 2:
            find_min_max();
            break;
        case 3:
            linear_search();
            break;
        case 4:
            differentiate_function();
            break;
        case 5:
            set_calculation_parameters();
            break;
        case 6:
            break;
        case 7:
            free_memory();
            printf("Программа завершена.\n");
            return 0;
        default:
            printf("Неверный выбор. Попробуйте снова.\n");
        }

        /* Если еще не рассчитывали, предложим сделать расчет */
        if (!results_exist && choice != 5 && choice != 6) {
            printf("\nДанные еще не рассчитаны. Выполнить расчет? (1-да, 0-нет): ");
            int calc_choice;
            scanf("%d", &calc_choice);
            if (calc_choice == 1) {
                calculate_functions();
            }
        }
    }
}
