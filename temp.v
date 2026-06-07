fn temp(int a, int b) int {
	return a + b
}

fn someFunc(int abs, int ok) int {
	return temp(1 + 2 + temp(1,2), 0) + 10
}

fn main() int {
	int a = 10 + 50 + 5.0
	a = 1
	temp(0,0)
	int c = 10 + temp(1 + 2, 2 + 3) + 1 + someFunc(a + 1, 0)
}
