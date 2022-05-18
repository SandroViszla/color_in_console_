#include <iostream>
#include <sstream>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>


namespace ESC_STR {
    enum console_state_t {
        CON_TO_END         = 0,
        CON_FROM_BEGIN     = 1,
        CON_WHOLE          = 2
    };

    enum font_state_t {
        FONT_NORMAL        = 0,
        FONT_PARTLY_BOLD   = 4,
        FONT_GLOW          = 5,
        FONT_INVERSE       = 7
    };

    class ESCstream {
    private:
        std::string _esc;
    public:
        ESCstream( std::string esc ) : _esc( esc ) {};

        friend std::ostream& operator<<( std::ostream&, ESCstream );
    };

    std::ostream& operator<<( std::ostream& stream, ESCstream source ) {
        return stream << source._esc;
    }

    ESCstream POS( int x, int y ) {
        std::ostringstream _esc_string;
        _esc_string << "\E[" << x << ";" << y << "H";

        return ESCstream(_esc_string.str() );
    }

    ESCstream COLOR( int color_id ) {
        std::ostringstream _esc_string;
        _esc_string << "\E[" << color_id << "m";

        return ESCstream(_esc_string.str() );
    }

    ESCstream HIGHLIGHT( font_state_t state ) {
        std::ostringstream _esc_string;
        _esc_string << "\E[" << state << "m";

        return ESCstream(_esc_string.str() );
    }

    ESCstream CLEAR( console_state_t state ) {
        std::ostringstream _esc_string;
        _esc_string << "\E[" << state << "j";

        return ESCstream(_esc_string.str() );
    }

    std::ostream &CLEAR( std::ostream &out ) {
        return out << std::string( "\E[K" );
    }

    ESCstream SPEED( unsigned int seconds ) {
        sleep( seconds );
        return ESCstream( "" );
    }

    ESCstream USPEED( useconds_t useconds ) {
        usleep( useconds );
        return ESCstream( "" );
    }
}


static bool is_done = false;
static const uint8_t tab = 2;

int flag_status() {
    char buf[ 512 ];
    int n = 0, flags = fcntl( 0, F_GETFL );

    usleep( 1 );

    fcntl( 0, F_SETFL, flags | O_NONBLOCK );
    n = read( 0, buf, 512 );
    fcntl( 0, F_SETFL, flags );

    return n;
}

void handler( int signal_id ) {
    is_done = signal_id;
}

int main( int args, char **argv ) {
    signal( SIGINT, handler );

    bool _is_bright = false;
    int _color_id = 0;

    struct winsize _window = {};
    ioctl( 0, TIOCGWINSZ, &_window );

    while ( !is_done ) {
        for ( uint64_t i = 0; i <= _window.ws_col / 2 && !is_done; i += tab ) {
            std::cout << ESC_STR::COLOR( ( _is_bright ? 100 : 40 ) + _color_id );
            for ( uint64_t j = 0; j < _window.ws_row && !is_done; j += 1 ) {
                if ( flag_status() > 0 ) is_done = true;

                ioctl( 0, TIOCGWINSZ, &_window );
                std::cout << ESC_STR::POS( j, _window.ws_col / 2 + i ) << "  "
                          << ESC_STR::POS( j, _window.ws_col / 2 - i ) << "  "
                          << ESC_STR::USPEED( 5000 ) << std::endl;
            }

            if ( ++_color_id > 7 ) {
                _color_id = 0;
                _is_bright = !_is_bright;
            }
        }
    }

    std::cout << ESC_STR::POS( 0, 0 ) << ESC_STR::CLEAR( ESC_STR::CON_WHOLE );

    return 0;
}