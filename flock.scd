SynthDef("sonic-pi-flock",
             {|bufnum = 0,
    	note = 52, amp = 1, amp_slide = 0, amp_slide_shape = 1, amp_slide_curve = 0,
	pan = 0, pan_slide = 0, pan_slide_shape = 1, pan_slide_curve = 0,
	attack = 0, decay = 0, sustain = 0, release = 1,
	attack_level = 1, decay_level = -1, sustain_level = 1, env_curve = 1,
    out_bus = 0 |

               var snd, env, freq;

	decay_level = Select.kr(decay_level < 0, [decay_level, sustain_level]);
	amp = amp.varlag(amp_slide, amp_slide_curve, amp_slide_shape);
	pan = pan.varlag(pan_slide, pan_slide_curve, pan_slide_shape);
    freq = note.midicps;
    	env = Env.new([0, attack_level, decay_level, sustain_level, 0],
		[attack, decay, sustain, release],
		env_curve);

   snd = SinOsc.ar([freq,freq],0,0.5);
   snd = Flock(bufnum, snd);
   snd = snd * EnvGen.kr(env, doneAction: 2);
    Out.ar(out_bus, snd)}
    ).writeDefFile("/Users/ethan/Desktop/sc-synths");