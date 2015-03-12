BINDIR=		.
NOMAN=		noman
WARNINGS=	yes
CFLAGS+=	-I${PLAN9}/include -DPLAN9PORT
LDFLAGS+=	-L${PLAN9}/lib
LDADD+=		-lmemdraw -lmemlayer -ldraw -l9
PROG=		maze
SRCS=		maze.c

.include <bsd.prog.mk>
