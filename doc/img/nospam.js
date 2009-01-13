function nospam(addr, dom, tld, linkOnly) {
    var st = addr + "@" + dom + "." + tld;
    if (linkOnly == true) {
        document.write("<a href='mailto:" + st + "'>");
    } else {
        document.write("- <a href='mailto:" + st + "'>" + st + "</a>.");
    }
}
