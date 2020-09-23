#include "homewidget.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QPushButton>
#include <QPainter>
#include <QLabel>
#include <QIcon>
#include <QDebug>
#include <QTimer>

HomeWidget::HomeWidget(QWidget *parent)
    : QFrame(parent)
{
    layout = new QHBoxLayout();

    status_label = new QLabel("当前状态:");
    status_label->setObjectName("status_label");

    status_text = new QLabel(" 已经停止 ");
    status_text->setObjectName("status_text");

    start_button = new QToolButton();
    start_button->setMinimumSize(60,90);
    start_button->setObjectName("start");
    start_button->setCheckable(true);
    start_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    start_button->setIcon(QIcon(":/image/start.png"));
    start_button->setIconSize(QSize(50,50));
    start_button->setText("启动");

    reset_button = new QToolButton();
    reset_button->setMinimumSize(60,90);
    reset_button->setObjectName("reset");
    reset_button->setCheckable(false);
    reset_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    reset_button->setIcon(QIcon(":/image/reset.png"));
    reset_button->setIconSize(QSize(50,50));
    reset_button->setText("复位");
    reset_button->setEnabled(false);

    adjust_button = new QToolButton();
    adjust_button->setMinimumSize(60,90);
    adjust_button->setObjectName("adjust");
    adjust_button->setCheckable(true);
    adjust_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    adjust_button->setIcon(QIcon(":/image/adjust.png"));
    adjust_button->setIconSize(QSize(50,50));
    adjust_button->setText("校准");
    adjust_button->setEnabled(false);

    left_rotate_button = new QToolButton();
    left_rotate_button->setMinimumSize(60,90);
    left_rotate_button->setObjectName("left rotate");
    left_rotate_button->setCheckable(true);
    left_rotate_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    left_rotate_button->setIcon(QIcon(":/image/lrotate.png"));
    left_rotate_button->setIconSize(QSize(50,50));
    left_rotate_button->setText("左旋");
    left_rotate_button->setEnabled(false);

    right_rotate_button = new QToolButton();
    right_rotate_button->setMinimumSize(60,90);
    right_rotate_button->setObjectName("right rotate");
    right_rotate_button->setCheckable(true);
    right_rotate_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    right_rotate_button->setIcon(QIcon(":/image/rrotate.png"));
    right_rotate_button->setIconSize(QSize(50,50));
    right_rotate_button->setText("右旋");
    right_rotate_button->setEnabled(false);

    auto_button = new QToolButton();
    auto_button->setMinimumSize(60,90);
    auto_button->setObjectName("auto");
    auto_button->setCheckable(true);
    auto_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    auto_button->setIcon(QIcon(":/image/auto.png"));
    auto_button->setIconSize(QSize(50,50));
    auto_button->setText("自动");
    auto_button->setDisabled(true);

    status_layout = new QHBoxLayout();
    status_layout->setAlignment(Qt::AlignLeft);
    status_layout->addWidget(status_label);
    status_layout->addSpacing(10);
    status_layout->addWidget(status_text);

    control_layout = new QHBoxLayout();
    control_layout->setAlignment(Qt::AlignLeft);
    control_layout->addWidget(start_button);
    control_layout->addWidget(reset_button);
    control_layout->addWidget(adjust_button);
    control_layout->addWidget(left_rotate_button);
    control_layout->addWidget(right_rotate_button);
    control_layout->addWidget(auto_button);

    layout_left = new QVBoxLayout();
    layout_left->setAlignment(Qt::AlignTop);
    layout_left->addSpacing(5);
    layout_left->addLayout(status_layout);
    layout_left->addSpacing(5);

    QFrame *line0 = new QFrame();
    line0->setFrameShape(QFrame::HLine);
    layout_left->addWidget(line0);

    layout_left->addSpacing(5);
    layout_left->addLayout(control_layout);
    layout_left->addSpacing(5);

    QFrame *line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    layout_left->addWidget(line1);

//    layout_right = new QVBoxLayout();
//    video1 = new VideoWidget();
//    video2 = new VideoWidget();
//    layout_right->addWidget(video1);

//    QFrame *line2 = new QFrame();
//    line2->setFrameShape(QFrame::HLine);
//    layout_right->addWidget(line2);
//    layout_right->addWidget(video2);

    layout->addLayout(layout_left);
    QFrame *line3 = new QFrame();
    line3->setFrameShape(QFrame::VLine);
    layout->addWidget(line3);
//    layout->addLayout(layout_right);
    this->setLayout(layout);

    connect(start_button, &QToolButton::clicked, this, &HomeWidget::on_start_button_clicked);
    connect(reset_button, &QToolButton::clicked, this, &HomeWidget::on_reset_button_clicked);
    connect(adjust_button, &QToolButton::clicked, this, &HomeWidget::on_adjust_button_clicked);
//    connect(auto_button, &QToolButton::clicked, this, &HomeWidget::on_auto_button_clicked);
}

HomeWidget::~HomeWidget()
{
    delete status_label;
    delete status_text;
    delete start_button;
    delete reset_button;
    delete adjust_button;
    delete left_rotate_button;
    delete right_rotate_button;

    delete auto_button;
    delete status_layout;
    delete control_layout;
    delete layout_left;

//    delete layout_right;
//    delete video1;
//    delete video2;

    delete layout;
}

void HomeWidget::timer_stop()
{
    set_adjust_button_status(true);
}

void HomeWidget::set_start_button_status(bool ok)
{
    start_button->setEnabled(ok);
}

void HomeWidget::set_reset_button_status(bool ok)
{
    reset_button->setEnabled(ok);
}

void HomeWidget::set_adjust_button_status(bool ok)
{
    adjust_button->setEnabled(ok);
    left_rotate_button->setEnabled(ok);
    right_rotate_button->setEnabled(ok);
    auto_button->setEnabled(ok);
}

void HomeWidget::on_start_button_clicked(bool checked)
{
    if(checked){
        start_button->setText("停止");
        start_button->setIcon(QIcon(":/image/stop.png"));
        status_text->setText(" 正在运行 ");
        status_text->setStyleSheet("color: green");
    }else{
        start_button->setText("启动");
        start_button->setIcon(QIcon(":/image/start.png"));
        status_text->setText(" 已经停止 ");
        status_text->setStyleSheet("color: red");
        reset_button->setEnabled(false);
        set_adjust_button_status(false);
    }
}

void HomeWidget::on_reset_button_clicked(bool checked)
{

}

void HomeWidget::on_adjust_button_clicked(bool checked)
{
    if(checked){
        adjust_button->setText("继续");
        status_text->setText(" 手动调整机械臂到初始位置 ");
        set_adjust_button_status(false);
        QTimer::singleShot(3000, this, SLOT(timer_stop()));
    }else{
        adjust_button->setText("校准");
        status_text->setText(" 正在运行 ");
    }
}

//void HomeWidget::on_auto_button_clicked(bool checked)
//{
//    if(checked){
//        auto_button->setText("手动");
//        auto_button->setIcon(QIcon(":/image/hand.png"));
//    }else{
//        auto_button->setText("自动");
//        auto_button->setIcon(QIcon(":/image/auto.png"));
//    }
//}

void HomeWidget::resizeEvent(QResizeEvent *event)
{
    (void)event;
}

VideoWidget::VideoWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{

}

VideoWidget::~VideoWidget()
{
    if(texture) {
        texture->destroy();
        delete texture;
    }
}

void VideoWidget::initShaders()
{
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl");
    program.link();
    program.bind();
}

void VideoWidget::initTextures()
{
    texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture->setFormat(QOpenGLTexture::RGBFormat);
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::Repeat);
    texture->setSize(704, 576);
    texture->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::UInt8);
}

void VideoWidget::on_get_frame(char *dat, int w, int h)
{
    w=h;
    h=w;
    texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (const void *)dat);
    update();
}

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};

void VideoWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    vertex_buffer.create();

    VertexData vertices[] = {
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D( 0.0f, 1.0f)},
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D( 1.0f, 1.0f)},
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D( 0.0f, 0.0f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D( 1.0f, 0.0f)}
    };

    vertex_buffer.bind();
    vertex_buffer.allocate(vertices, 4 * sizeof(VertexData));
}

void VideoWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();
    program.setUniformValue("texture", 0);

    vertex_buffer.bind();

    program.enableAttributeArray("a_position");
    program.setAttributeBuffer("a_position", GL_FLOAT, 0, 3, sizeof(VertexData));

    program.enableAttributeArray("a_texcoord");
    program.setAttributeBuffer("a_texcoord", GL_FLOAT, sizeof(QVector3D), 2, sizeof(VertexData));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    (void)e;
    cnt %= 4;
    cnt++;

    qDebug() << texture->isCreated() << texture->width() << texture->height();

//    if(!texture->isCreated()){

//    }

//    if(texture->width() != 600){
//        texture->setSize(600, 500);
//        texture->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::UInt8);
//    }

    QString s = QString(":/image/12%1.jpg").arg(cnt);
    qDebug() << s;
    QImage img = QImage(s).convertToFormat(QImage::Format_RGB888);
    texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (const void *)img.constBits());
    update();
}
