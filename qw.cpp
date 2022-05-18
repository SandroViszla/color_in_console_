#include <iostream> // cin, cout
#include <sstream> // i(o)stringsteam, str
#include <iomanip> // flush, endl
#include <sys/ioctl.h> // winsize, (tty_)ioctl
#include <fcntl.h> // fcntl, O_NONBLOCK, F_S(G)ETFL
#include <stdio.h> // EOF
#include <stdlib.h> // (s)rand
#include <signal.h> // signal, SIGINT
#include <unistd.h> // getpid, read

static bool done = false;

using namespace std; // Standard inout

namespace con {

    int comax()
    {
        struct winsize w;
        ioctl(0, TIOCGWINSZ, &w);
        return(w.ws_col);
    }

    int romax()
    {
        struct winsize w;
        ioctl(0, TIOCGWINSZ, &w);
        return(w.ws_row);
    }

    ostream& ED(ostream& s){return s << string("\033[2J");}
    ostream& EL(ostream& s) {return s << string("\033[K");}

    class estream
    {
    private:
        string escape; // Escape string
    public:
        estream(string e) : escape(e) {};
        friend ostream& operator << (ostream&, estream);
    };

    ostream& operator << (ostream& s, estream e)
{
    s << e.escape << flush;
    return s;
}

estream CUP(int y, int x)
{
    ostringstream sout; // output string stream
    sout << "\033[" << y << ";" << x << "H"; // gotoxy ESC
    return estream(sout.str()); // return ESC object
}
estream SGR(int r)
{
    ostringstream sout;
    sout << "\033[" << r << "m";
    return estream(sout.str());
}

}

using con::SGR;
using con::CUP;

void interruptor(int signo)
{
    done = signo;
    return;
}
int kbin()
{
    char buf[ 512 ];
    int n = 0, flags = fcntl( 0, F_GETFL );

    usleep( 1 );

    fcntl( 0, F_SETFL, flags | O_NONBLOCK );
    n = read( 0, buf, 512 );
    fcntl( 0, F_SETFL, flags );

    return n;
}

int main()
{
    cout << con::CUP(con::romax(), 1);
    cout << con::ED << "^C or Enter to exit" << flush;
    cout << con::CUP(con::romax()-1, 1);
    //cout << con::romax() << "x" << con::comax() << flush;
    int x=0, y=0; // cursor location
    int romax = con::romax();
    int comax = con::comax();
    int background;
    int b;
    srand(getpid());
    signal(SIGINT, interruptor);
    while(!done) {
        for (int i = 0; (x + y) != (comax + romax - 1) && !done; i++) {
            x = 1;
            y = i + 1;
            while ((y > 0) && (x < comax + 1) && (!done)) {
                if (kbin() > 0) {
                    done = true;
                }

                b = 40 + i % 7;
                if (y <= romax - 1) {
                    cout << SGR(b) << CUP(romax - y, comax - x + 1);
                    cout << ' ' << flush;
                }
                usleep(1000);
                x++;
                y--;
            }
        }
    }
        cout << CUP(0, 0) << SGR(0) << con::ED;
        return 0;
}

