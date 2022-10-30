def fibonacci( n ):
    if (n <= 1):
        return n
    return fibonacci(n - 2) + fibonacci(n - 1);

for i in range(39):
    fibonacci(i)

