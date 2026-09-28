#include "snake.h"

#include <QApplication>
#include <QPainter>
#include <QKeyEvent>
#include <QTimerEvent>
#include <QRandomGenerator>
#include <QtMath>

//Matrix3x3 

Matrix3x3::Matrix3x3() {
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            m[i][j] = (i == j) ? 1.0 : 0.0;
}

Matrix3x3 Matrix3x3::identity() { return Matrix3x3(); }

Matrix3x3 Matrix3x3::rotation(double a) {
    Matrix3x3 r;
    const double c = qCos(a), s = qSin(a);
    r.m[0][0] =  c;  r.m[0][1] = -s;  r.m[0][2] = 0.0;
    r.m[1][0] =  s;  r.m[1][1] =  c;  r.m[1][2] = 0.0;
    r.m[2][0] = 0.0; r.m[2][1] = 0.0; r.m[2][2] = 1.0;
    return r;
}

Matrix3x3 Matrix3x3::translation(double tx, double ty) {
    Matrix3x3 t;             // единичная матрица
    t.m[0][2] = tx;
    t.m[1][2] = ty;
    return t;
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3& o) const {
    Matrix3x3 r;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            r.m[i][j] = 0.0;
            for (int k = 0; k < 3; ++k)
                r.m[i][j] += m[i][k] * o.m[k][j];
        }
    return r;
}

QPointF Matrix3x3::apply(const QPointF& p) const {
    const double x = m[0][0]*p.x() + m[0][1]*p.y() + m[0][2];
    const double y = m[1][0]*p.x() + m[1][1]*p.y() + m[1][2];
    return QPointF(x, y);
}

//Snake 

Snake::Snake(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Змейка");
    setFixedSize(COLS * DOT, ROWS * DOT);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, &Snake::scoreChanged, this,
            [this](int s){ setWindowTitle(QString("Змейка — счёт: %1").arg(s)); });
    loadImages();
    initGame();
}

void Snake::loadImages() {
    imgHead  = QImage("head.png");
    imgDot   = QImage("dot.png");
    imgApple = QImage("apple.png");
}

void Snake::initGame() {
    score = 0;
    gameOver = false;
    angle = 0.0; // движение вдоль +X

    const double cx = (COLS / 2) * DOT;
    const double cy = (ROWS / 2) * DOT;

    snake.clear();
    // Голова + два сегмента позади — вдоль направления -X
    for (int i = 0; i < 3; ++i)
        snake.append(QPointF(cx - i * DOT, cy));

    locateApple();
    timer.start(100, this);
}

void Snake::locateApple() {
    bool ok;
    do {
        apple = QPointF(QRandomGenerator::global()->bounded(COLS) * DOT + DOT/2.0,
                        QRandomGenerator::global()->bounded(ROWS) * DOT + DOT/2.0);
        ok = true;
        for (const QPointF& s : snake) {
            const QPointF d = s - apple;
            if (qSqrt(d.x()*d.x() + d.y()*d.y()) < DOT) { ok = false; break; }
        }
    } while (!ok);
}

// Перемещение змейки через аффинные преобразования:
void Snake::move() {
    const Matrix3x3 R = Matrix3x3::rotation(angle);       // матрица поворота
    const QPointF stepVec = R.apply(QPointF(DOT, 0.0));   // вектор шага

    const Matrix3x3 T = Matrix3x3::translation(stepVec.x(), stepVec.y());

    for (int i = snake.size() - 1; i > 0; --i)
        snake[i] = snake[i - 1];

    snake[0] = T.apply(snake[0]);
}

bool Snake::checkCollision() {
    const QPointF h = snake[0];

    // Стены
    if (h.x() < 0 || h.x() > COLS * DOT ||
        h.y() < 0 || h.y() > ROWS * DOT)
        return true;

    // Тело 
    for (int i = 1; i < snake.size(); ++i) {
        const QPointF d = snake[i] - h;
        if (qSqrt(d.x()*d.x() + d.y()*d.y()) < DOT * 0.5)
            return true;
    }
    return false;
}

void Snake::timerEvent(QTimerEvent *e) {
    if (e->timerId() != timer.timerId() || gameOver) return;

    // Сьела ли змея яблоко
    const QPointF d = snake[0] - apple;
    if (qSqrt(d.x()*d.x() + d.y()*d.y()) < DOT) {
        snake.append(snake.last());
        locateApple();
        emit scoreChanged(++score);
    }

    move();

    if (checkCollision()) { gameOver = true; timer.stop(); }
    repaint();
}

void Snake::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(25, 25, 25));

    if (gameOver) {
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter,
                   "Game Over\nПробел — начать заново");
        return;
    }

    // Яблоко
    if (!imgApple.isNull()) {
        p.drawImage(QRectF(apple.x() - DOT/2.0, apple.y() - DOT/2.0,
                           DOT, DOT), imgApple);
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::red);
        p.drawEllipse(apple, DOT/2.0, DOT/2.0);
    }

    // Тело
    for (int i = 1; i < snake.size(); ++i) {
        if (!imgDot.isNull()) {
            p.drawImage(QRectF(snake[i].x() - DOT/2.0, snake[i].y() - DOT/2.0,
                               DOT, DOT), imgDot);
        } else {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(30, 130, 30));
            p.drawEllipse(snake[i], DOT/2.0, DOT/2.0);
        }
    }

    // Голова — поворачивается на текущий угол движения
    if (!imgHead.isNull()) {
        p.save();
        p.translate(snake[0]);
        p.rotate(qRadiansToDegrees(angle));
        p.drawImage(QRectF(-DOT/2.0, -DOT/2.0, DOT, DOT), imgHead);
        p.restore();
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(60, 200, 60));
        p.drawEllipse(snake[0], DOT/2.0, DOT/2.0);
    }
}

void Snake::keyPressEvent(QKeyEvent *e) {
    const int k = e->key();

    if (gameOver) {
        if (k == Qt::Key_Space) initGame();
        return;
    }

    // Шаг поворота — 15°
    const double ROT_STEP = M_PI / 12.0;

    double delta = 0.0;
    if      (k == Qt::Key_Left)  delta = -ROT_STEP;
    else if (k == Qt::Key_Right) delta =  ROT_STEP;
    else return;

    // Изменение направления
    const Matrix3x3 R = Matrix3x3::rotation(delta);
    const QPointF dirUnit(qCos(angle), qSin(angle));
    const QPointF newDir = R.apply(dirUnit);
    angle = qAtan2(newDir.y(), newDir.x());
}
