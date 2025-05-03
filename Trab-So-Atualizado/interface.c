#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include "aviao.h"

#define WIDTH 500
#define HEIGHT 500
#define N 5

Aviao* frota;
int shm_id;
cairo_surface_t *aviao_img = NULL;






gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    // Fundo verde floresta
    cairo_set_source_rgb(cr, 0.13, 0.55, 0.13); 
    cairo_paint(cr);



    // Pista cinza no centro
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_arc(cr, WIDTH/2, HEIGHT/2, 5, 0, 2 * G_PI);
    cairo_fill(cr);

   
    //Desenhando o avião em PNG
    for (int i = 0; i < N; i++) 
    {
    if (frota[i].status == STATUS_VOANDO) {
        float x = frota[i].x;
        float y = frota[i].y;
        float px = x * WIDTH;
        float py = y * HEIGHT;

        // Determina rotação conforme quadrante
        double rotacao = 0.0;
        if (x > 0.5 && y < 0.5) {           // Q1
            rotacao = -G_PI;
        } else if (x < 0.5 && y < 0.5) {    // Q2
            rotacao = -3 * G_PI / 2;
        } else if (x > 0.5 && y > 0.5) {    // Q4
            rotacao = -G_PI / 2;
        }
        
        cairo_save(cr);
        cairo_translate(cr, px, py);           // Move para o ponto do avião
        cairo_rotate(cr, rotacao);             // Aplica rotação
        cairo_translate(cr, -10, -10);         // Ajusta para desenhar do canto
        cairo_scale(cr, 20.0 / 440.0, 20.0 / 399.0);  // Reduz imagem
        cairo_set_source_surface(cr, aviao_img, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }

}   return FALSE;
}


gboolean redraw(gpointer user_data) {
    GtkWidget *widget = GTK_WIDGET(user_data);
    gtk_widget_queue_draw(widget);
    return TRUE; // continua chamando
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <shm_id>\n", argv[0]);
        return 1;
    }

    shm_id = atoi(argv[1]);
    frota = (Aviao*)shmat(shm_id, NULL, 0);
    if (frota == (void*) -1) {
        perror("Erro ao anexar memória");
        return 1;
    }

    gtk_init(&argc, &argv);



    aviao_img = cairo_image_surface_create_from_png("aviao.png");
    if (!aviao_img) {
    fprintf(stderr, "Erro ao carregar imagem aviao.png\n");
    return 1;
        }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Radar Aéreo");
    gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw), NULL);

    // Redesenha a cada 100ms
    g_timeout_add(100, redraw, drawing_area);

    gtk_widget_show_all(window);
    gtk_main();

    shmdt(frota);
    return 0;
}
