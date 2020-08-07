// without mul and add.
Flock : UGen {
    *ar { arg bufnum = 0, freq = 440.0, iphase = 0.0, seed = 0;
        ^this.multiNew('audio', bufnum, freq, iphase, seed)
    }
    *kr { arg bufnum = 0, freq = 440.0, iphase = 0.0, seed = 0;
        ^this.multiNew('control', bufnum, freq, iphase, seed)
    }
}