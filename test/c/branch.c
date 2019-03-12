unsigned branch(unsigned c, unsigned condition, unsigned n) {
  if (condition < 10)
    n = c + condition;
  else if (condition < 100)
    n = c * condition;
  else
    n = c - condition;
  return n;
}
