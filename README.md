![This project is not called Big Ben](https://upload.wikimedia.org/wikipedia/commons/a/a6/Big_Ben_Clock_Face.jpg)

chronos: the hands of time
==========================

Welcome to **`chronos: the hands of time`**. You may look at it and think that it is yet another implementation of a DateTime-style class written mostly as an excuse to practice language features. And it is, but there is *more* to it than that.

You see, long ago—before the advent of the digital display—there was the **analog clock**. And it was *good*. Once inducted into its mysteries, you could—with great effort—use it to tell the time of day. For you youngsters, I shall reveal the secret:  it had a big hand for hours and a little hand for minutes. Those were the selfsame, eponymous *hands of time*. Merely by observing their positions—carefully measuring their angles against the handy guide at the circumference—you could align yourself with the movement of the celestial bodies!

It is from this deep insight—and not any sort of trite [pop-culture](https://en.wikipedia.org/wiki/Manos:_The_Hands_of_Fate) reference—that `chronos: the hands of time` came to be. Like the analog clock of yore, it has a big hand and a little hand. One indicates seconds (measured in seconds), the other indicates subseconds (measured in picoseconds, naturally). Together, they tell the time. And date, actually.

You'll see. It'll be amazing. Or, at any rate, it can't possibly be any worse than that [`std::chrono`](https://en.cppreference.com/w/cpp/header/chrono) monstrosity. Look at how the ANSI committee left off the all-important `s` at the end. Not only does the letter signify the plural, making it inherently better than the singular, but it adds cachet to it by alluding to Greek [philosophy](https://en.wikipedia.org/wiki/Chronos) (not [mythology](https://en.wikipedia.org/wiki/Cronus)), and does so in a startlingly [original](http://lmgtfy.com/?q=chronos+the+hands+of+time) manner.

If I was any more pretentious, I'd have named this library [Musica Universalis](https://en.wikipedia.org/wiki/Musica_universalis), but `mu` is too short to make a good namespace. It also brings back sad memories of Lemuria and Atlantis (hey, I'm even older than I look!), so you were spared. As it was, I almost went with [`aion`](https://en.wikipedia.org/wiki/Aion_(deity)) or even [`kairos`](https://en.wikipedia.org/wiki/Kairos). It was a near thing, but I decided they were too obscure, and I did not respect you enough to take the risk that you would fail to understand the reference.

This is also why I went with `chronos` as the namespace, instead of confusing you with `cthot`, but CtHoT is still the official  acronym for the project. (All projects need an official acronym: trust me on this.) Besides, you might confuse it with Cthulhu. I suppose that, if I had wanted to guarantee recognition by pandering to you nerds, I could have gone with `gallifrey`. Or, if I was an anglophile, there was always `bigben`. 

But I didn't and that's that. The die is cast; there's no going back in time and changing it. At least not until version 2.0, which features extreme git integration.


