#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

typedef struct Mnode Mnode;
typedef enum Orient
{
	NORTH = 0,
	EAST,
	SOUTH,
	WEST
} Orient;

struct Mnode
{
	Mnode*	back;
	Point	p;
	Orient	orient;
	int	tried;
	int	open;
};

enum
{
	F = 1<<0,
	L = 1<<1,
	R = 1<<2
};

#define ALL (F|L|R)

Mnode**	maze;
u64int	pathlen;
int	x, y;

Mnode*	makemaze(int);
void	drawmaze(Mnode*);
Mnode*	newmnode(Mnode*, Orient);
Mnode*	backup(Mnode*);
Orient	nextorient(Orient, int);
Point	nextpos(Mnode*, Orient);
int	collides(Point);
int	onedge(Point);
void	usage(char*);

void
main(int argc, char* argv[])
{
	Mnode	*end;

	x = 20;
	y = 20;
	ARGBEGIN{
	case 'x':
		x = atoi(ARGF());
		break;
	case 'y':
		y = atoi(ARGF());
		break;
	default:
		usage(argv0);
		break;
	}ARGEND
	maze = calloc(x*y, sizeof(Mnode *));
	srand(truerand());
	end = makemaze(nrand(y));
	drawmaze(end);
	exits(0);
}

Mnode*
makemaze(int ystart)
{
	Mnode*		cur, *far;
	static Mnode	dummy;
	Point		p;
	Orient		orient;
	u64int	 	max;
	int		move;

	max = 0;
	move = 0;
	dummy.p.x = -1;
	dummy.p.y = ystart;
	dummy.back = nil;
	far = cur = newmnode(&dummy, EAST);
	while(cur->back != nil){
		if(cur->tried == ALL)
			cur = backup(cur);
		switch(nrand(3)){
		case 0:
			move = F;
			break;
		case 1:
			move = L;
			break;
		case 2:
			move = R;
			break;
		default:
			fprint(2, "Illegal random");
			exits("Illegal random");
			break;
		}
		if(cur->tried & move)
			continue;
		cur->tried |= move;
		orient = nextorient(cur->orient, move);
		p = nextpos(cur, orient);
		if(collides(p))
			continue;
		cur->open |= move;
		cur = newmnode(cur, orient);
		if(++pathlen > max){
			max = pathlen;
			far = cur;
		}
	}
	while(!onedge(far->p))
		far = far->back;
	return far;
}

Mnode*
newmnode(Mnode* prev, Orient orient)
{
	Mnode *new;
	Point p;
	static int count;

	p = nextpos(prev, orient);
	new = malloc(sizeof(Mnode));
	new->p = p;
	new->orient = orient; 
	new->tried = 0;
	new->open = 0;
	new->back = prev;
	maze[p.x + p.y * x] = new;
	return new;
}

Mnode*
backup(Mnode* end)
{
	for(; end->tried == ALL; end = end->back)
		pathlen--;
	return end;
}

Orient
nextorient(Orient old, int way)
{
	Orient new;

	new = 0;
	switch (way) {
	case F:
		new = old;
		break;
	case L:
		new = (old + 3) % 4;
		break;
	case R:
		new = (old + 1) % 4;
		break;
	default:
		fprint(2, "Illegal orientation");
		exits("Illegal orientation");
		break;
	}
	return new;
}

Point
nextpos(Mnode* m, Orient o)
{
	Point new;

	switch (o) {
	case NORTH:
		new.y = m->p.y - 1;
		new.x = m->p.x;
		break;
	case EAST:
		new.x = m->p.x + 1;
		new.y = m->p.y;
		break;
	case SOUTH:
		new.y = m->p.y + 1;
		new.x = m->p.x;
		break;
	case WEST:
		new.x = m->p.x - 1;
		new.y = m->p.y;
		break;
	default:
		fprint(2, "nextpos: Illegal direction");
		exits("Illegal direction");
		break;
	}
	return new;
}

int
collides(Point p)
{
	if((p.x < 0) || (p.y < 0) || (p.x >= x) || (p.y >= y))
		return 1;
	else 
		return maze[p.x + p.y * x] ? 1 : 0;
}

int
onedge(Point p)
{
	return (p.x == 0) || (p.y == 0) || (p.x == x - 1) || (p.y == y - 1);
}

void
usage(char* name)
{
	fprint(2, "usage: %s [-x width] [-y height]\n", name);
	exits("usage");
}

Memimage*	allocpix(ulong);
void		drawmnode(Mnode *);
void		drawside(Point, Orient, Memimage *);
void		mazeline(Point, Point, Point, Memimage *);

Memimage*	black, *red, *canvas;
Point		shift = { 5,  5 };
#define D	20

void
drawmaze(Mnode* end)
{
	Mnode*	m;
	int	i, j;

	memimageinit();
	black = allocpix(DBlack);
	red = allocpix(DRed);
	canvas = allocmemimage(Rect(0,0,D*x + 10,D*y + 10), RGB24);
	memfillcolor(canvas, DWhite);
	end->open = ALL;
	for(i = 0; i < x; i++){
		for(j = 0; j < y; j++){
			if(m = maze[i + j * x])
				drawmnode(m);
			else{
				fprint(2, "Missing Mnode");
				exits("Missing Mnode");
			}
		}
	}
	writememimage(1, canvas);
	for(; end->back != nil; end = end->back){
		mazeline(end->p, end->back->p, addpt(shift, Pt(10, 10)), red);
	}
	writememimage(3, canvas);
	freememimage(black);
	freememimage(red);
	freememimage(canvas);
}

void
drawmnode(Mnode* m)
{
	if((m->open & F) == 0)
		drawside(m->p, nextorient(m->orient, F), black);
	if((m->open & L) == 0)
		drawside(m->p, nextorient(m->orient, L), black);
	if((m->open & R) == 0)
		drawside(m->p, nextorient(m->orient, R), black);
}

void
drawside(Point p, Orient o, Memimage *color)
{
	switch (o) {
	case NORTH:
		mazeline(addpt(p, Pt(0,0)), addpt(p, Pt(1,0)), shift, color);
		break;
	case EAST:
		mazeline(addpt(p, Pt(1,0)), addpt(p, Pt(1,1)), shift, color);
		break;
	case SOUTH:
		mazeline(addpt(p, Pt(0,1)), addpt(p, Pt(1,1)), shift, color);
		break;
	case WEST:
		mazeline(addpt(p, Pt(0,0)), addpt(p, Pt(0,1)), shift, color);
		break;
	default:
		fprint(2, "drawside: illegal direction");
		exits("illegal direction");
		break;
	}
}

void
mazeline(Point s, Point d, Point tr, Memimage *color)
{
	Point st, dt;

	st = addpt(mulpt(s, D), tr);
	dt = addpt(mulpt(d, D), tr);
	memimageline(canvas, st, dt, Enddisc, Enddisc, 0, color, ZP, SoverD);
}

Memimage *
allocpix(ulong color)
{
	Memimage* m;

	m = allocmemimage(Rect(0, 0, 1, 1), RGB24);
	memfillcolor(m, color);
	m->clipr = Rect(-D,-D,D,D);
	m->flags = Frepl;
	return m;
}
