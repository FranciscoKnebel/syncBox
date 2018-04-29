# syncBox

[![Build Status](https://travis-ci.com/FranciscoKnebel/syncBox.svg?token=XTCGVcTeCasm1L7c4fss&branch=master)](https://travis-ci.com/FranciscoKnebel/syncBox)

[![US](assets/flags/eua.gif)]()
Inspired by Dropbox application, developed as a project for the Operating Systems II (INF01151) class.

[![BR](assets/flags/br.gif)]()
Aplicação inspirada no Dropbox, desenvolvida como projeto da disciplina de Sistemas Operacionais II (INF01151).

## Building

```
make util
Builds the utility objects, used by both the server and the client.
```

```
make client
Builds the client module objects, bundling them into binary file "dst/dropboxClient".
```

```
make server
Builds the server module objects, bundling them into binary file "dst/dropboxServer".
```

Running `make` will build all the project files and generate both the client and server applications.

## Using
After building the application, both applications will be stored in the `dst` folder.
Arguments for both applications are explained on execution.


## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/FranciscoKnebel/syncBox/tags).

## Authors

<table style="text-align: center;">
  <th>Contributors</th>
  <th>Contributions</th>
  <tr>
    <td>
      <img src="https://avatars.githubusercontent.com/FranciscoKnebel?s=75">
      <br>
      <a href="https://github.com/FranciscoKnebel">Francisco Knebel</a>
    </td>
    <td>
      <a href="https://github.com/FranciscoKnebel/syncBox/commits?author=FranciscoKnebel">Contributions</a> by FranciscoKnebel
    </td>
  </tr>
  <tr>
    <td>
      <img src="https://avatars.githubusercontent.com/lzancan?s=75">
      <br>
      <a href="https://github.com/lzancan">Luciano Zancan</a>
    </td>
    <td>
      <a href="https://github.com/FranciscoKnebel/syncBox/commits?author=lzancan">Contributions</a> by lzancan
    </td>
  </tr>
</table>

See also the full list of [contributors](https://github.com/FranciscoKnebel/syncBox/contributors) who participated in this project.

## License

This project is licensed under the _MIT_ License - see the [LICENSE.md](LICENSE.md) file for details.
