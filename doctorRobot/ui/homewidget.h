#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QFrame>
#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

class MainWindow;
class QImage;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QPushButton;

class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    VideoWidget(QWidget *parent = 0);
    ~VideoWidget();

    void initShaders();
    void initTextures();

public slots:
    void on_get_frame(char *dat, int w, int h);

protected:
    void initializeGL();
    void paintGL();

private:
    int width = 0, height = 0, cnt = 0;
    QOpenGLShaderProgram program;
    QOpenGLTexture *texture = nullptr;
    QOpenGLBuffer vertex_buffer;

protected:
    void mouseDoubleClickEvent(QMouseEvent *);
};

class HomeWidget : public QFrame
{
    Q_OBJECT

friend class MainWindow;

public:
    HomeWidget(QWidget *parent = 0);
    ~HomeWidget();

public slots:
    void timer_stop();
    void set_start_button_status(bool ok);
    void set_reset_button_status(bool ok);
    void set_adjust_button_status(bool ok);

    void on_start_button_clicked(bool checked);
    void on_reset_button_clicked(bool checked);
    void on_adjust_button_clicked(bool checked);

//    void on_auto_button_clicked(bool checked);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QHBoxLayout *layout;

    QVBoxLayout *layout_left;
    QHBoxLayout *status_layout;
    QHBoxLayout *control_layout;
    QLabel *status_label;
    QLabel *status_text;

    QToolButton *start_button;
    QToolButton *reset_button;
    QToolButton *adjust_button;
    QToolButton *left_rotate_button;
    QToolButton *right_rotate_button;
    QToolButton *auto_button;

    QVBoxLayout *layout_right;
    VideoWidget *video1;
    VideoWidget *video2;
};
#endif // HomeWIDGET_H
