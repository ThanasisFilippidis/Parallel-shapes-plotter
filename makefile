SHAPES = shapes.o
CIRCLE = circle.o
SEMICIRCLE = semicircle.o
RING = ring.o
SQUARE = square.o
ELLIPSE = ellipse.o
OUTSHAPES = shapes
OUTCIRCLE = circle
OUTSEMICIRCLE = semicircle
OUTRING = ring
OUTSQUARE = square
OUTELLIPSE = ellipse

CC = gcc
FLAGS  = -c -g
all: $(OUTSHAPES) $(OUTCIRCLE) $(OUTSEMICIRCLE) $(OUTRING) $(OUTSQUARE) $(OUTELLIPSE)

$(OUTSHAPES): $(SHAPES)
	$(CC) -g $(SHAPES) -o $(OUTSHAPES)

$(OUTCIRCLE): $(CIRCLE)
	$(CC) -g $(CIRCLE) -o $(OUTCIRCLE)

$(OUTSEMICIRCLE): $(SEMICIRCLE)
	$(CC) -g $(SEMICIRCLE) -o $(OUTSEMICIRCLE)

$(OUTRING): $(RING)
	$(CC) -g $(RING) -o $(OUTRING)

$(OUTSQUARE): $(SQUARE)
	$(CC) -g $(SQUARE) -o $(OUTSQUARE)

$(OUTELLIPSE): $(ELLIPSE)
	$(CC) -g $(ELLIPSE) -o $(OUTELLIPSE)

shapes.o: shapes.c
	$(CC) $(FLAGS) shapes.c

circle.o: circle.c
	$(CC) $(FLAGS) circle.c

semicircle.o: semicircle.c
	$(CC) $(FLAGS) semicircle.c

ring.o: ring.c
	$(CC) $(FLAGS) ring.c

square.o: square.c
	$(CC) $(FLAGS) square.c

ellipse.o: ellipse.c
	$(CC) $(FLAGS) ellipse.c

clean:
	rm -f $(OUTSHAPES) $(SHAPES) $(OUTCIRCLE) $(CIRCLE) $(OUTSEMICIRCLE) $(SEMICIRCLE) $(OUTRING) $(RING)
	rm -f $(OUTSQUARE) $(SQUARE) $(OUTELLIPSE) $(ELLIPSE)
