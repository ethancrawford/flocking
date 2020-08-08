// without mul and add.
Flock : UGen {
    *ar { arg bufnum = 0, freq = 440.0, iphase = 0.0, seed = 0, num_boids = 20;
        ^this.multiNew('audio', bufnum, freq, iphase, seed, num_boids)
    }
    *kr { arg bufnum = 0, freq = 440.0, iphase = 0.0, seed = 0, num_boids = 20;
        ^this.multiNew('control', bufnum, freq, iphase, seed, num_boids)
    }
}