use std::fs::File;

use fourchan_sdk::{
    core::{
        html::{html_parse_post, TextType},
        repository::{get_catalog, get_catalog_json, get_thread, get_thread_json},
    },
    models::catalog::Catalog,
    models::post::Post,
    models::thread::Thread,
    traits::operations::Operations,
};

fn post_to_str(post: &Post, board: &str) -> String {
    let mut result: String = String::new();

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
        match post_text {
            Ok(vec) => {
                for texttype in vec {
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
                        TextType::Code(t) => {
                            result.push_str(t.as_str());
                        }
                    }
                }
            }
            Err(e) => {
                println!("Parsing error: {}", e);
            }
        }
    }

    result.push_str("\n\n");

    return result;
}

fn thread_to_str(thread: &Thread, board: &str) -> String {
    let mut result: String = String::new();

    for post in &thread.posts {
        let post_str = post_to_str(post, board);
        result.push_str(&post_str);
    }

    return result;
}

fn catalog_to_str(catalog: &Catalog, board: &str) -> String {
    let mut result: String = String::new();

    for catalog_item in catalog {
        for post in catalog_item.threads.iter() {
            let post_str = post_to_str(post, board);
            result.push_str(&post_str);
        }
    }

    return result;
}

fn main() {
    let args = clap::App::new("4chan-downloader")
        .version("1.0")
        .author("Romeu Vieira <romeu.bizz@gmail.com>")
        .about("4Chan Downloader")
        .arg(
            clap::Arg::with_name("Json")
                .short("j")
                .long("json")
                .help("Output as JSON"),
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
            clap::Arg::with_name("Images")
                .short("i")
                .long("images")
                .value_name("DIR")
                .help("Get images")
                .takes_value(true),
        )
        .get_matches();

    let arg_has_json = args.is_present("Json");
    let arg_has_board = args.is_present("Board");
    let arg_has_thread = args.is_present("Thread");
    let arg_has_image = args.is_present("Images");

    let arg_board = args.value_of("Board");
    let arg_thread = args.value_of("Thread");
    let arg_images = args.value_of("Images");

    if arg_has_board {
        let board = arg_board.unwrap();

        if arg_has_thread {
            let thread = arg_thread.unwrap();

            if arg_has_json {
                let result = get_thread_json(board, thread);
                match result {
                    Ok(s) => {
                        println!("{}", s);
                    }
                    Err(e) => {
                        eprintln!("{}", e);
                    }
                }
            } else {
                let result = get_thread(board, thread);
                match result {
                    Ok(t) => {
                        if arg_has_image {
                            let image_dir = arg_images.unwrap();

                            for post in t.posts.into_iter() {
                                if post.filename.is_some() {
                                    match post.get_file_url(board) {
                                        Ok(url) => {
                                            let filename = post.filename.unwrap();
                                            let extension = post.ext.unwrap();

                                            let path =
                                                format!("{}/{}{}", image_dir, filename, extension);
                                            let img_bytes = reqwest::blocking::get(url)
                                                .unwrap()
                                                .bytes()
                                                .unwrap();
                                            let mut cursor = std::io::Cursor::new(img_bytes);

                                            let mut file = File::create(&path)
                                                .expect("File could not be created");
                                            match std::io::copy(&mut cursor, &mut file) {
                                                Ok(_) => {
                                                    println!("Downloaded file: {}", path);
                                                }
                                                Err(e) => {
                                                    eprintln!("{}", e);
                                                }
                                            }
                                        }
                                        Err(e) => {
                                            eprintln!("{}", e);
                                        }
                                    }
                                }
                            }
                        } else {
                            let thread_text = thread_to_str(&t, &board);
                            println!("{}", thread_text);
                        }
                    }
                    Err(e) => {
                        eprintln!("{}", e);
                    }
                }
            }
        } else {
            if arg_has_json {
                let result = get_catalog_json(board);
                match result {
                    Ok(s) => {
                        println!("{}", s);
                    }
                    Err(e) => {
                        eprintln!("{}", e);
                    }
                }
            } else {
                let result = get_catalog(board);
                match result {
                    Ok(c) => {
                        let catalog_text = catalog_to_str(&c, &board);
                        println!("{}", catalog_text);
                    }
                    Err(e) => {
                        eprintln!("{}", e);
                    }
                }
            }
        }
    }
}
