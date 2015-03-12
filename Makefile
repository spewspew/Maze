BINDIR=		.
NOMAN=		noman
WARNINGS=	yes
CFLAGS+=	-I${PLAN9}/include
LDFLAGS+=	-L${PLAN9}/lib
LDADD+=		-l draw -l memdraw -l 9 
PROG=		maze
SRCS=		maze.c

.include <bsd.prog.mk>
