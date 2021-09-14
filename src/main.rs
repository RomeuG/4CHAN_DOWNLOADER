use fourchan_sdk::{
    core::{
        html::{html_parse_post, TextType},
        repository::{self, get_catalog, get_catalog_json, get_thread, get_thread_json},
    },
    models::thread::Thread,
};

fn thread_to_str(thread: &Thread, board: &str) -> String {
    let mut result: String = String::new();

    for post in &thread.posts {
        // post name
        if let Some(name) = &post.name {
            let _name = format!("{} ", name);
            result.push_str(_name.as_str());
        }

        // post tripcode
        if let Some(tripcode) = &post.trip {
            let _tripcode = format!("{} ", tripcode);
            result.push_str(_tripcode.as_str());
        }

        // post file
        if let Some(filename) = &post.filename {
            let w = &post.w.unwrap();
            let h = &post.h.unwrap();
            let ext = post.ext.as_ref().unwrap();
            let date = &post.now;

            let ftext = format!("{}{} ({}x{}) {} ", filename, ext, w, h, date);
            result.push_str(ftext.as_str());

            let post_number = format!("no.{}\n", &post.no);
            result.push_str(post_number.as_str());

            let furl = format!(
                "Media: http://i.4cdn.org/{}/{}{}\n",
                board,
                &post.tim.unwrap(),
                ext
            );
            result.push_str(furl.as_str());
        } else {
            let post_number = format!("no.{}\n", &post.no);
            result.push_str(post_number.as_str());
        }

        if let Some(title) = &post.sub {
            let _title = format!("Title: {}\n", title);
            result.push_str(_title.as_str());
        }

        if let Some(text) = &post.com {
            let post_text = html_parse_post(&text);
            for texttype in post_text {
                match texttype {
                    TextType::NewLine => {
                        result.push_str("\n");
                    }
                    TextType::Link(t, _) => {
                        result.push_str(&t);
                    }
                    TextType::Quote(t) | TextType::PlainText(t) => {
                        result.push_str(&t);
                    }
                    TextType::Italics(t) => {
                        let _italics = format!("/{}/", &t);
                        result.push_str(_italics.as_str());
                    }
                }
            }
        }

        result.push_str("\n\n");
    }

    return result;
}

fn main() {
    let args = clap::App::new("newsboat-archiver")
        .version("1.0")
        .author("Romeu Vieira <romeu.bizz@gmail.com>")
        .about("Archive Newsboat DB information")
        .arg(
            clap::Arg::with_name("Json")
                .short("j")
                .long("json")
                .help("Output as JSON"),
        )
        .arg(
            clap::Arg::with_name("Catalog")
                .short("c")
                .long("catalog")
                .value_name("CATALOG")
                .help("Get catalog")
                .takes_value(true),
        )
        .arg(
            clap::Arg::with_name("Board")
                .short("b")
                .long("board")
                .value_name("BOARD")
                .help("4chan board")
                .takes_value(true),
        )
        .arg(
            clap::Arg::with_name("Thread")
                .short("t")
                .long("thread")
                .value_name("THREAD")
                .help("Thread id")
                .takes_value(true),
        )
        .arg(
            clap::Arg::with_name("File")
                .short("f")
                .long("file")
                .value_name("FILE")
                .help("Get file")
                .takes_value(true),
        )
        .arg(
            clap::Arg::with_name("Images")
                .short("i")
                .long("images")
                .value_name("DIR")
                .help("Get images")
                .takes_value(true),
        )
        .get_matches();

    let arg_has_json = args.is_present("Json");
    let arg_has_catalog = args.is_present("Catalog");
    let arg_has_board = args.is_present("Board");
    let arg_has_thread = args.is_present("Thread");

    let arg_catalog = args.value_of("Catalog");
    let arg_board = args.value_of("Board");
    let arg_thread = args.value_of("Thread");
    let arg_file = args.value_of("File");
    let arg_images = args.value_of("Images");

    if arg_has_catalog {
        let catalog = arg_catalog.unwrap();

        if arg_has_json {
            let result = get_catalog_json(catalog);
            println!("{}", result);
        } else {
            let result = get_catalog(catalog);
            // display as image board in a string
        }
    } else if arg_has_thread {
        let board = arg_board.unwrap();
        let thread = arg_thread.unwrap();

        if arg_has_json {
            let result = get_thread_json(board, thread);
            println!("{}", result);
        } else {
            let result = get_thread(board, thread);
            let thread_text = thread_to_str(&result, &board);
            println!("{}", thread_text);
        }
    }
}
