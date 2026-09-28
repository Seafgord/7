#ifndef SNAKE_H
#define SNAKE_H

#include <QWidget>
#include <QImage>
#include <QVector>
#include <QPointF>
#include <QBasicTimer>

static const int DOT  = 10;
static const int COLS = 30;
static const int ROWS = 30;

// Простая 3x3-матрица для аффинных преобразований на плоскости
class Matrix3x3 {
public:
    double m[3][3];

    Matrix3x3();
    static Matrix3x3 identity();
    static Matrix3x3 rotation(double angleRad);
    static Matrix3x3 translation(double tx, double ty);

    Matrix3x3 operator*(const Matrix3x3& other) const;
    QPointF   apply(const QPointF& p) const;
};

class Snake : public QWidget {
    Q_OBJECT
public:
    explicit Snake(QWidget *parent = nullptr);

signals:
    void scoreChanged(int score);

protected:
    void loadImages();
    void initGame();
    void locateApple();
    void move();
    bool checkCollision();
    void timerEvent(QTimerEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    QImage imgHead, imgDot, imgApple;
    QVector<QPointF> snake;   // центры сегментов; snake[0] — голова
    QPointF apple;            // центр яблока
    QBasicTimer timer;
    int    score    = 0;
    double angle    = 0.0;    // направление движения головы
    bool   gameOver = false;
};

#endif
