#include <LedControl.h>
#include <LiquidCrystal.h>
#include <binary.h>
#include <time.h>

#define DELAY_TIME_DECREASE 0.3
#define SCORE_TARGET_INCREASE 1.2
#define CLEANED_ROWS_AT_FAIL 2
#define SCORE_LEVEL_INCREASE 0.5

LedControl lc=LedControl(12,11,10,1);
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const short nrRows = 9, nrColumns = 10, MATRIX_SIZE = 8;
const int JOYSTICK_READ_X = 1, JOYSTICK_READ_Y = 2, JOYSTICK_MAX_READ = 1023;

short matrix[nrRows][nrColumns];
int nrPointsPerRow[MATRIX_SIZE];

void initMatrix() {
  for (int i = 0; i < nrRows - 1; ++i)
    for (int j = 1; j < nrColumns - 1; ++j)
      matrix[i][j] = 0;
  for (int i = 0; i < nrColumns; ++i)
    matrix[nrRows - 1][i] = -1;
  for (int i = 0; i < nrRows; ++i)
    matrix[i][0] = matrix[i][nrColumns - 1] = -1;
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    nrPointsPerRow[i] = 0;
  }
}

enum MoveDirection {
  None = 0,
  Right = 1,
  Left = 2
};

enum TriangleDiagonalStyle {
  DiagonalBefore = 0,
  DiagonalAfter = 1
};

enum TrianglePointPosition {
  AboveDiagonal = 0,
  UnderDiagonal = 1
};

enum TetrisObjectType {
  PointType = 0,
  LineType = 1,
  TriangleType = 2,
  SquareType = 3
};

class Point {
private:
  int x, y;
public:
  Point(int _x = 0, int _y = 0) : x(_x), y(_y) { };
  int getX() const;

  void setX(const int _x);

  int getY() const;

  void setY(const int _y);

  static int sortFunctionDesc(const void * Point1, const void * Point2);
};

int Point::getX() const {
  return x;
}

void Point::setX(const int _x) {
  x = _x;
}

int Point::getY() const {
  return y;
}

void Point::setY(const int _y) {
  y = _y;
}

static int Point::sortFunctionDesc(const void * Point1, const void * Point2) {
    Point p1 = *(Point*)Point1;
    Point p2 = *(Point*)Point2;
    if (p1.x == p2.x)
      return p2.y - p2.y;
    return p2.x - p1.x;
}

class TetrisObject {
protected:
  int nrPoints;
  Point* points;

public:
  TetrisObject(int _nrPoints, Point* _points) : nrPoints(_nrPoints) {
    points = new Point[_nrPoints];
    for (int i = 0; i < _nrPoints; ++i) {
      points[i] = _points[i];
    }
  }

  TetrisObject(const TetrisObject &to) : nrPoints(to.nrPoints) {
    points = new Point[to.nrPoints];
    for (int i = 0; i < to.nrPoints; ++i) {
      points[i] = to.points[i];
    }
  }

  const TetrisObject& operator=(TetrisObject& to) {
    if (points)
      delete[] points;
    points = new Point[to.nrPoints];
    for (int i = 0; i < to.nrPoints; ++i) {
      points[i] = to.points[i];
    }
    nrPoints = to.nrPoints;
    return *this;
  }

  ~TetrisObject() {
    delete[] points;
  }

  int getNrPoints() const {
    return nrPoints;
  }

  void setNrPoints(const int _nrPoints) {
    nrPoints = _nrPoints;
  }

  Point* getPoints() {
    Point* _points = new Point[nrPoints];
    for (int i = 0; i < nrPoints; ++i) {
      _points[i] = points[i];
    }
    return _points;
  }

  void move(MoveDirection _direction = MoveDirection::None);
  void lightUp();
  void lightDown();
  bool isMovePossible(MoveDirection _direction = MoveDirection::None);
  bool isStopped();
  void appear();
  void dispose();
};

void TetrisObject::lightUp() {
  for (int i = 0; i < nrPoints; ++i)  
    if (points[i].getX() >= 0 && points[i].getY() >= 0) {
      lc.setLed(0, points[i].getX(), points[i].getY() - 1, true);
      matrix[points[i].getX()][points[i].getY()] = 1;
    }
}

void TetrisObject::lightDown() {
  for (int i = 0; i < nrPoints; ++i)
    if (points[i].getX() >= 0 && points[i].getY() >= 0) {
      lc.setLed(0, points[i].getX(), points[i].getY() - 1, false);
      matrix[points[i].getX()][points[i].getY()] = 0;
    }
}

void TetrisObject::move(MoveDirection _direction = MoveDirection::None) {
  if (_direction == MoveDirection::None) {
    lightDown();
    for (int i = 0; i < nrPoints; ++i)
      points[i].setX(points[i].getX() + 1); 
    lightUp();
  } else if (_direction == MoveDirection::Left) {
    lightDown();
    for (int i = 0; i < nrPoints; ++i) 
      points[i].setY(points[i].getY() - 1);
    lightUp();
  } else {
    lightDown();
    for (int i = 0; i < nrPoints; ++i) 
      points[i].setY(points[i].getY() + 1);
    lightUp();
  }
}

void TetrisObject::dispose() {
  for (int i = 0; i < nrPoints; ++i) {
    matrix[points[i].getX()][points[i].getY()] = -1;
    ++nrPointsPerRow[points[i].getX()];
  }
}

void TetrisObject::appear() {
  qsort(points, nrPoints, sizeof(Point), Point::sortFunctionDesc);
  int lastLine = points[0].getX();
  for (int i = 0; i < nrPoints; ++i) {
    int x = points[i].getX();
    int y = points[i].getY();
    if (lastLine != x) {
      break;
    }
    if (matrix[0][y] != -1) {
      lc.setLed(0, 0, y - 1, true);
      matrix[0][y] = 1;
    }
  }
}

bool TetrisObject::isMovePossible(MoveDirection _direction = MoveDirection::None) {
  if (_direction == MoveDirection::None) {
    for (int i = 0; i < nrPoints; ++i) {
      if (matrix[points[i].getX() + 1][points[i].getY()] == -1)
        return false;
    } 
    return true;
  } else if (_direction == MoveDirection::Left) {
    for (int i = 0; i < nrPoints; ++i) {
      if (points[i].getY() <= 1 || matrix[points[i].getX()][points[i].getY() - 1] == -1)
        return false;
    }
    return true;
  } else if (_direction == MoveDirection::Right) {
    for (int i = 0; i < nrPoints; ++i) {
      if (points[i].getY() == nrColumns - 2 || matrix[points[i].getX()][points[i].getY() + 1] == -1)
        return false;
    }
    return true;
  }
}

bool TetrisObject::isStopped() {
  return !isMovePossible(MoveDirection::None);
}

class PointObject : public TetrisObject {
public:
  PointObject(Point point)
    : TetrisObject(1, &point) { };  

  PointObject(const PointObject &to) : TetrisObject(to) { }; 
};

class LineObject : public TetrisObject {
public:
  LineObject(Point* point)
    : TetrisObject(2, point) { };  

  LineObject(const LineObject &to) : TetrisObject(to) { }; 
};

class TriangleObject : public TetrisObject {
public:
  TriangleObject(Point* point)
    : TetrisObject(3, point) { };  

  TriangleObject(const TriangleObject &to) : TetrisObject(to) { }; 
};

class SquareObject : public TetrisObject {
public:
  SquareObject(Point* point)
    : TetrisObject(4, point) { };  

  SquareObject(const SquareObject &to) : TetrisObject(to) { }; 
};

int getDirectionFromJoyStick() {
  double y = (double)analogRead(JOYSTICK_READ_Y) / JOYSTICK_MAX_READ;
  if (y > 0.65)
    return MoveDirection::Right;
  if (y < 0.35)
    return MoveDirection::Left;
  return MoveDirection::None;
}

class TetrisObjectBuilder {
private:
  static TetrisObjectBuilder* builder;
  TetrisObjectBuilder() { };
public:

  ~TetrisObjectBuilder() {
    delete builder;
    builder = NULL;
  };

  TetrisObject* createPointObject(int column);
  TetrisObject* createLineObject(int column);
  TetrisObject* createTriangleObject(int column, TriangleDiagonalStyle diagonalStyle = TriangleDiagonalStyle::DiagonalAfter, 
                                     TrianglePointPosition pointPosition = TrianglePointPosition::AboveDiagonal);
  TetrisObject* createSquareObject(int column);
  static TetrisObjectBuilder& getBuilder();
  TetrisObject* createRandomTetrisObject();
};

TetrisObjectBuilder* TetrisObjectBuilder::builder = NULL;

TetrisObjectBuilder& TetrisObjectBuilder::getBuilder() {
  if (!builder)
    builder = new TetrisObjectBuilder();
  return *builder;
}

TetrisObject* TetrisObjectBuilder::createPointObject(int column) {
  Point p(0, column);
  TetrisObject* po = new PointObject(p);
  return po; 
}

TetrisObject* TetrisObjectBuilder::createLineObject(int column) {
  Point* points = new Point[2] { Point(0, column), Point(0, column + 1) };
  TetrisObject* lo = new LineObject(points);
  delete[] points;
  return lo; 
}

TetrisObject* TetrisObjectBuilder::createTriangleObject(int column, 
                                                        TriangleDiagonalStyle diagonalStyle = TriangleDiagonalStyle::DiagonalAfter, 
                                                        TrianglePointPosition pointPosition = TrianglePointPosition::AboveDiagonal) {
  Point* points = new Point[3];
  points[0] = Point(-1, column);
  short _position = pointPosition == TrianglePointPosition::AboveDiagonal ? -1 : 0;
  if (diagonalStyle == TriangleDiagonalStyle::DiagonalAfter) {
    points[1] = Point(0, column + 1);
    points[2] = Point(_position, column - _position);
  } else {
    points[1] = Point(0, column - 1);
    points[2] = Point(_position, column + _position); 
  }
  TetrisObject* to = new TriangleObject(points);
  delete[] points;
  return to; 
}

TetrisObject* TetrisObjectBuilder::createSquareObject(int column) {
  Point* points = new Point[4];
  points[0].setX(-1);
  points[0].setY(column);
  points[1].setX(-1); 
  points[1].setY(column + 1);
  points[2].setX(0);
  points[2].setY(column);
  points[3].setX(0);
  points[3].setY(column + 1);
  TetrisObject* so = new SquareObject(points);
  delete[] points;
  return so; 
}

TetrisObject* TetrisObjectBuilder::createRandomTetrisObject() {
  short tetrisObjectType = random(4);
  switch (tetrisObjectType) {
    case TetrisObjectType::PointType:
      return createPointObject(random(1, 8));
    case TetrisObjectType::LineType:
      srand(time(NULL));
      return createLineObject(random(1, 7));
    case TetrisObjectType::TriangleType:
      {
        short diagonalPosition = random(2);
        short pointPosition = random(2);
        int column;
        if (diagonalPosition) 
          column = random(1, 7);
        else
          column = random(2, 8);
        return createTriangleObject(column, diagonalPosition, pointPosition);
      } 
    default:
      return createSquareObject(random(1, 8));
  }
}

class LevelManager {
private:
  int currentLevel;
  int scoreTarget, levelScore;
  int delayTime;
public:
  LevelManager(int _currentLevel = 1, int _scoreTarget = 150, int _delayTime = 500, int _levelScore = 50)
    : currentLevel(_currentLevel), scoreTarget(_scoreTarget), delayTime(_delayTime), levelScore(_levelScore) { };

  int getCurrentLevel() const {
    return currentLevel;
  }

  int getScoreTarget() const {
    return scoreTarget;
  }

  int getDelayTime() const {
    return delayTime;
  }

  int getLevelScore() const {
    return levelScore;
  }

  void nextLevel() {
    ++currentLevel;
    delayTime -= DELAY_TIME_DECREASE * delayTime;
    scoreTarget += SCORE_TARGET_INCREASE * scoreTarget;
    levelScore += SCORE_LEVEL_INCREASE * levelScore;
    lcd.setCursor(0, 0);
    lcd.print("Level ");
    lcd.print(getCurrentLevel());
  }

  void reset() {
    currentLevel = 1; 
    scoreTarget = 150;
    delayTime = 500;
    levelScore = 50;
  }
};

class Player {
private:
  int remainingLives;
  int score;
public:
  Player(int _remainingLives = 3, int _score = 0) : remainingLives(_remainingLives), score(score) { };

  void setRemainingLives(const int lives) {
    remainingLives = lives;
  }

  int getRemainingLives() const {
    return remainingLives;
  }

  void setScore(const int _score) {
    score = _score;
  }

  int getScore() const {
    return score;
  }

  void increaseScore(const int amount);
  void decreaseScore(const int amount);
  void decreaseRemainingLives();
  void increaseRemainingLives();
  void reinit();
};

void Player::increaseScore(const int amount) {
  score += amount;
  lcd.setCursor(0, 1);
  lcd.print("Score ");
  lcd.print(getScore());
}

void Player::decreaseScore(const int amount) {
  score -= amount;
  lcd.setCursor(0, 1);
  lcd.print("Score ");
  lcd.print(getScore());
}

void Player::increaseRemainingLives() {
  ++remainingLives;
}

void Player::decreaseRemainingLives() {
  --remainingLives;
}

void Player::reinit() {
  score = 0;
  remainingLives = 3;
}

class TetrisGameManager {
private:
  static TetrisGameManager* instance;
  TetrisObjectBuilder* tetrisObjectBuilder;  
  LevelManager* levelManager;
  Player* player;
  bool gameStarted;
  
  TetrisGameManager() { 
    tetrisObjectBuilder = &TetrisObjectBuilder::getBuilder();
    levelManager = new LevelManager();
    player = new Player();
    gameStarted = false;
  };

  void getMatrixDown(int row);
  void endGame();
  
public:
  static TetrisGameManager& getManager();
  
  ~TetrisGameManager() {
    delete instance;
    delete levelManager;
    delete tetrisObjectBuilder;
    delete player;
    instance = NULL;
  }

  void loopGame();
  void startGame();
  bool isGameStarted();
  bool isFullColumn();
  int getFullRow();
  void clearRowInMatrix(int row);
  void removeLife();
};

int TetrisGameManager::getFullRow() {
  for (int i = 0; i < MATRIX_SIZE; ++i)
    if (nrPointsPerRow[i] == MATRIX_SIZE)
      return i;
  return -1;
}

bool TetrisGameManager::isFullColumn() {
  for (int i = 0; i < MATRIX_SIZE; ++i)
    if (matrix[0][i + 1] == -1)
      return true;
  return false;
}

void TetrisGameManager::clearRowInMatrix(int row) {
  lc.setRow(0, row, B00000000);
  delay(levelManager->getDelayTime() / 2);
  lc.setRow(0, row, B11111111); 
  delay(levelManager->getDelayTime() / 2);
  lc.setRow(0, row, B00000000);
  getMatrixDown(row); 
}

void TetrisGameManager::getMatrixDown(int row) {
  if (row == 0)
    return;
  for (int i = row - 1; i >= 0; --i) {
    for (int j = 0; j < MATRIX_SIZE; ++j) {
      if (matrix[i][j + 1])
        lc.setLed(0, i + 1, j, true);
      else
        lc.setLed(0, i + 1, j, false);
      matrix[i + 1][j + 1] = matrix[i][j + 1];
      nrPointsPerRow[i + 1] = nrPointsPerRow[i];
    }
  }
  for (int i = 0; i < MATRIX_SIZE; ++i) {
    matrix[0][i + 1] = 0;
  }
  lc.setRow(0, 0, B00000000);
  nrPointsPerRow[0] = 0;
}

void TetrisGameManager::removeLife() {
  if (player->getRemainingLives() == 0)
    endGame();
  else {
    player->decreaseRemainingLives();
    for (int i = 0; i < CLEANED_ROWS_AT_FAIL; ++i)
      clearRowInMatrix(MATRIX_SIZE - 1);
  }
}

bool TetrisGameManager::isGameStarted() {
  return gameStarted;
}

void TetrisGameManager::startGame() {
  gameStarted = true;
  randomSeed(time(NULL));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level ");
  lcd.print(levelManager->getCurrentLevel());
  lcd.setCursor(0, 1);
  lcd.print("Score ");
  lcd.print(player->getScore());
}

void TetrisGameManager::endGame() {
  gameStarted = false;
  for (int i = 0; i < MATRIX_SIZE; ++i) 
    lc.setRow(0, i, B00000000);
  initMatrix();
  player->reinit();
  levelManager->reset();
  lcd.clear();
  lcd.print("Start Game!");
}

void TetrisGameManager::loopGame() {
  TetrisObject* to = tetrisObjectBuilder->createRandomTetrisObject();
  to->appear();
  delay(levelManager->getDelayTime());
  while (!to->isStopped()) {
    MoveDirection _direction = getDirectionFromJoyStick();
    if (to->isMovePossible(_direction) && !to->isStopped()) {
      delay(levelManager->getDelayTime());
      to->move(_direction);
    }
  }
  to->dispose();
  delete to;
  int fullRow = getFullRow();
  if (fullRow != -1) {
    player->increaseScore(levelManager->getLevelScore());
    if (player->getScore() >= levelManager->getScoreTarget())
      levelManager->nextLevel();
    clearRowInMatrix(fullRow);
  }
  if (isFullColumn())
    removeLife();
}

TetrisGameManager* TetrisGameManager::instance = NULL;

TetrisGameManager& TetrisGameManager::getManager() {
  if (instance == NULL)
    instance = new TetrisGameManager;
  return *instance;
}

TetrisGameManager* manager;

void setup() {
  // put your setup code here, to run once:
  //set a medium brightness for the Leds
  lc.shutdown(0,false);// turn off power saving, enables display
  lc.setIntensity(0,8);// sets brightness (0~15 possible values)
  lc.clearDisplay(0);
  initMatrix();
  manager = &TetrisGameManager::getManager();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Start Game!");
}

void loop() {
  if (manager->isGameStarted())
    manager->loopGame();
  MoveDirection _direction = getDirectionFromJoyStick();
  if (_direction != MoveDirection::None && !manager->isGameStarted())
    manager->startGame();
}
