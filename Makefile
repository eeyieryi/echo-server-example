all: echo echo-multi echo-go

echo: echo.c
	cc -o echo echo.c

echo-multi: echo-multi.c
	cc -o echo-multi echo-multi.c

echo-go: echo.go
	go build -o echo-go echo.go
