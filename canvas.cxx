#include "canvas.hxx"
#include <random>

std::random_device rd;
std::mt19937 mrand(rd());
qreal n;
int cols, rows, w, active;
struct Cell;
Cell * current;
QVector<Cell *> cells;
QStack<Cell *> visited;
Canvas * canvas = nullptr;
QPainter * scene = new QPainter;
QImage * save = nullptr;

int index(int x, int y) {
  if (x < 0 || y < 0 || x >= cols || y >= rows)
    return -1;
  return x + y * cols;
}

struct Cell
{
    int i,j,x,y;
    bool inactive = false;
    bool visited = false;
    QVector<QLine> walls;

    Cell(int i, int j) :
      i(i), j(j), x(i*w), y(j*w),
      // Top Right Bottom Left
      walls({{x,y,x+w,y},
    {x+w,y,x+w,y+w},
    {x,y+w,x+w,y+w},
    {x,y,x,y+w}})
    {  }
    void show()
    {
      if (!inactive && visited) {
        scene->save();
        scene->setPen(Qt::NoPen);
        scene->setBrush(QBrush(Qt::gray));
        scene->drawRect(x,y,w,w);
        scene->restore();
      } scene->drawLines(walls);
    }

    QVector<QPair<int, Cell *> > checkNeighbors()
    {
      QVarLengthArray<int> indexes {
        index(i,j-1),
            index(i+1,j),
            index(i,j+1),
            index(i-1,j)
      }; QVector<QPair<int, Cell *> > neighbors;
      for (int i = 0, var = indexes[i];
           i < indexes.length();
           var = indexes[++i])
      {
        if (var != -1 && !cells[var]->visited)
          neighbors.push_back(qMakePair(i,cells.value(var)));
      }
      return neighbors;
    }
    void setInActive()
    {
      this->inactive = true;
      this->visited = true;
      this->walls.clear();
    }
};

void setup(int start)
{
  w = (canvas->width() > canvas->height() ? canvas->height() : canvas->width())/start;
  cols = canvas->width()/w;
  rows = canvas->height()/w;
  cells.reserve(cols*rows);
  for (int j = 0; j < rows; ++j)
    for (int i = 0; i < cols; ++i)
      cells.push_back(new Cell(i,j));
  current = cells[index(cols/2,rows/2)];
  current->visited = true;
  auto sgn = [](qreal t)->int {
    return qFuzzyCompare(1 + 0.0, 1 + t) ? 0 : 1 + t < 1 + 0.0 ? -1 : 1;
  }; int a = cols/2, b = rows/2;
  for (qreal angle = 0.01; angle < 2*M_PI; angle+=0.01) {
    int x = qFloor(qPow(qAbs(qFastCos(angle)), 2/n) * a * sgn(qFastCos(angle))),
        y = qFloor(qPow(qAbs(qFastSin(angle)), 2/n) * b * sgn(qFastSin(angle)));
    cells[index(x+(cols/2), y+(rows/2))]->setInActive();
  }
}

void out(QPaintDevice * p)
{
  scene->begin(p);
  scene->setPen(Qt::NoPen);
  scene->setBrush(Qt::black);
  scene->drawRect
      (0, 0, canvas->width(), canvas->height());
  scene->translate((canvas->width()-(w*cols))/2,(canvas->height()-(w*rows))/2);
  scene->setPen(Qt::white);
  scene->setBrush(Qt::white);
  scene->drawRect
      (0, 0, cols*w, rows*w);
  for (int i = 0; i < cells.length(); ++i) {
    cells[i]->show();
  } if (current) {
    scene->setPen(Qt::NoPen);
    scene->setBrush(QBrush(Qt::blue));
    scene->drawRect(current->x,current->y,w,w);
  } scene->end();
}

void draw()
{
  if (active) {
    QVector<QPair<int, Cell *> > neighbors = current->checkNeighbors();
    if (neighbors.length() > 0) {
      QPair<int, Cell *> neighbor = neighbors[mrand() % neighbors.length()];
      visited.push(current);
      current->walls[neighbor.first] = QLine();
      current = neighbor.second;
      current->walls[(neighbor.first+2)%4] = QLine();
      current->visited = true;
    } else if (visited.size() > 0) {
      current = visited.pop();
    } else {
      canvas->killTimer(active);
      active = 0;
      current = nullptr;
      if (save == nullptr) {
        save = new QImage(canvas->width(),canvas->height(), QImage::Format_ARGB32_Premultiplied);
        QString name(QString("%1.bmp").arg(QDateTime::currentMSecsSinceEpoch()));
        out(save);
        save->save(name);
        qDebug() << "Saved to:" << name;
      }
    } out(canvas);
  } else if (save){
    scene->begin(canvas);
    scene->drawImage(0,0,*save);
    scene->end();
  }
}












































































Canvas::Canvas(QWidget *parent)
  : QWidget(parent)
{
  this->setFixedSize(400,400);
  this->show();
  canvas = this;
  n = QInputDialog::getDouble(canvas, "Maze Generator 900", "Enter a bevel:", 2.0, 0, 10.0, 1);
  setup(QInputDialog::getInt(canvas, "Maze Generator 900", "Enter a grid size:", 2, 2, 100, 1));
  active = startTimer(QInputDialog::getDouble(canvas, "Maze Generator 900", "Enter a speed:", 16.67, 0, 1000, 2)); // Framerate (30fps,33.33ms) / (60fps,16.67ms)
}

void Canvas::paintEvent(QPaintEvent *)
{ draw(); }

void Canvas::timerEvent(QTimerEvent *)
{ update(); }
