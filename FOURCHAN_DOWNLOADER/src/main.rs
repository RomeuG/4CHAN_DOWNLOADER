use fourchan_sdk::core::repository::{self, get_catalog_json, get_thread_json};

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
                .long("directory")
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

    println!("arg_has_catalog = {}", arg_has_catalog);
    println!("arg_has_json = {}", arg_has_json);

    if arg_has_catalog {
        if arg_has_json {
            let catalog = arg_catalog.unwrap();
            let result = get_catalog_json(catalog);
            println!("{}", result);
        } else {
        }
    } else if arg_has_thread {
        if arg_has_json {
            let board = arg_board.unwrap();
            let thread = arg_thread.unwrap();
            let result = get_thread_json(board, thread);
            println!("{}", result);
        } else {
        }
    }

    // if let Some(c) = arg_catalog {
    //     // get catalog
    //     if arg_json {
    //         get_thread_json(arg_board, thread)
    //     } else {
    //     }
    // } else if let Some(t) = arg_thread {
    //     // get thread
    //     if arg_json {
    //     } else {
    //     }
    // }
}
